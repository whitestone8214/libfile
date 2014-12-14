/*
	LibFile - 파일 관련 함수 모음

	Copyright (C) 2014 Minho Jo <whitestone8214@openmailbox.org>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libtext.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

/* (malloc() 사용)
현재 위치를 반환합니다. */
char *libfile_here ();

/* (malloc() 사용)
홈 디렉토리를 반환합니다.
홈 디렉토리가 딱히 없는 경우 "/"를 반환합니다. */
char *libfile_home ();

/* (malloc() 사용)
주어진 파일 주소 'address'를 깔끔하게 고쳐 반환합니다. */
char *libfile_refine_address (const char *address);

/* (malloc() 사용)
주어진 파일(또는 디렉토리) 'address'가 속해 있는 디렉토리의 주소를 반환합니다.
"/"가 주어졌다면, 그냥 "/"를 반환합니다. */
char *libfile_parent (const char *address);

/* (malloc() 사용)
주어진 파일(또는 디렉토리) 'address'에서 이름 부분만 뚝 떼어 반환합니다.
"/"가 주어졌다면, 그냥 "/"를 반환합니다. */
char *libfile_name (const char *address);

/* (malloc() 사용)
주어진 파일(또는 디렉토리) 'address'에 대한 특정 정보를 반환합니다.
'category'에는 't'(파일 형식), 's'(크기), 'o'(소유자), 'g'(소유 그룹) 중 하나가 들어갈 수 있으며, 그 외의 것이 들어갈 경우 NULL을 반환합니다. */
char *libfile_status (const char *address, char category);

/* 'address'를 새로 만들며, 성공했을 경우 's'를, 실패했을 경우 'f'를 반환합니다.
'type'이 'f'인 경우 'address'는 일반 파일로, 'd'인 경우는 디렉토리로 만들어지며, 그 외의 값인 경우 그냥 'f'를 반환합니다.
'with_parents'가 'y'인 경우, 새로 만들고자 하는 파일이 속할 디렉토리가 없으면 그것도 새로 만듭니다.
이미 있는 파일인 경우 만들고자 하는 형식과 일치하면서 내용을 읽을 수 있으면 's'를, 그렇지 않으면 'f'를 반환합니다. */
char libfile_create (const char *address, char type, char with_parents);

/* (malloc() 사용)
파일 'address'의 내용 중 'from'번째 글자부터 'to'번째 글자까지를 반환합니다.
'from'은 0보다 작으면 0으로, 'to'는 마지막 글자의 위치 값보다 크면 마지막 글자의 위치 값으로 맞춰집니다.
'from'이 'to'보다 거나 파일 내용을 읽을 수 없으면 NULL을 반환합니다. */
char *libfile_read (const char *address, int from, int to);

/* 파일 'address'의 'where'번째 칸에 'content'를 넣으며, 성공했을 경우 's'를, 실패했을 경우 'f'를 반환합니다.
'where'은 0보다 작으면 0으로, 파일 내용의 마지막 글자 바로 다음의 위치 값보다 크면 마지막 글자 바로 다음의 위치 값으로 맞춰집니다.
'create'가 'y'인 경우는 파일 'address'가 없으면 새로 만듭니다. */
char libfile_write (const char *address, const char *content, int where, char create);

/* 파일 'from'을 'to'로 복사하며, 성공했을 경우 's'를, 실패했을 경우 'f'를 반환합니다.
'brake'는 char형 변수의 포인터이며, 가리키는 변수의 값이 0이 아닌 경우 작업이 중단됩니다.
복사하고자 하는 파일이 속할 디렉토리가 없으면 그 디렉토리를 새로 만듭니다. */
char libfile_copy (const char *from, const char *to, char *brake);

/* 파일 'address'를 삭제하며, 성공했을 경우 's'를, 실패했을 경우 'f'를 반환합니다.
'brake'는 char형 변수의 포인터이며, 가리키는 변수의 값이 0이 아닌 경우 작업이 중단됩니다.
삭제하고자 하는 파일이 디렉토리이고 안에 파일이 있으면 안에 있는 파일을 먼저 삭제합니다. */
char libfile_erase (const char *address, char *brake);

/* 파일 'from'을 'to'로 이동하며, 성공했을 경우 's'를, 실패했을 경우 'f'를 반환합니다.
'brake'는 char형 변수의 포인터이며, 가리키는 변수의 값이 0이 아닌 경우 작업이 중단됩니다. */
char libfile_move (const char *from, const char *to, char *brake);

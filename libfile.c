/*
	LibFile - 파일 관련 함수 모음

	Copyright (C) 2014-2015 Minho Jo <whitestone8214@openmailbox.org>

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

// gcc -shared $(pkg-config --libs libtext) -o libfile.so libfile.c

#include "libfile.h"

char *libfile_here () {
	char room[PATH_MAX]; getcwd (room, PATH_MAX); //if (getcwd (room, PATH_MAX) == NULL) return NULL;
	int length = 0; while (room[length] != '\0') length++;
	char *result = (char *) malloc (length + 1); if (result == NULL) return NULL;
	int nth; for (nth = 0; nth < length; nth++) result[nth] = room[nth]; result[nth] = '\0'; return result;
}

char *libfile_home () {if (getenv ("HOME") == NULL) return strdup ("/"); return strdup (getenv ("HOME"));}

char *libfile_refine_address (const char *address) {
	// Phase 1: Simple cases
	if (address == NULL) return NULL;
	if ((strcmp (address, "/") == 0) || (strcmp (address, "/.") == 0) || (strcmp (address, "/..") == 0)) return strdup ("/");
	if (strcmp (address, ".") == 0) return libfile_here ();
	if (strcmp (address, "~") == 0) return libfile_home ();
	if (strcmp (address, "..") == 0) {
		char *address1 = libfile_home (); if (address1 == NULL) return NULL;
		char *address2 = libtext_connect (2, address1, "/.."); free (address1); if (address2 == NULL) return NULL;
		address1 = libfile_refine_address (address2); free (address2); return address1;
	}
	
	// Phase 2: First character is not slash(/), Last character is slash(/)
	char *result = strdup (address); if (result == NULL) return NULL;
	if (result[0] != '/') {
		if ((result[0] == '~') && (result[1] == '/')) {
			char *address1 = libfile_home (); if (address1 == NULL) {free (result); return NULL;}
			char *address2 = libtext_cut (result, 0, 0, 'd'); free (result); if (address2 == NULL) {free (address1); return NULL;}
			result = libtext_connect (2, address1, address2); free (address2); free (address1); if (result == NULL) return NULL;
		}
		else {
			char *address1 = libfile_here (); if (address1 == NULL) {free (result); return NULL;}
			char *address2 = libtext_connect (3, address1, "/", result); free (address1); if (address2 == NULL) {free (result); return NULL;}
			free (result); result = strdup (address2); free (address2); if (result == NULL) return NULL;
		}
	}
	if (result[strlen (result) - 1] == '/') {
		char *address1 = libtext_cut (result, strlen (result) - 1, strlen (result) - 1, 'd'); if (address1 == NULL) {free (result); return NULL;}
		free (result); result = strdup (address1); free (address1); if (result == NULL) return NULL;
	}
	
	int nth = 0; while (nth >= 0) {
		if (nth >= strlen (result)) break;
		if ((result[nth] == '/') && (libtext_escaped (result, nth) == 'n')) {
			int nth1; for (nth1 = nth; nth1 < strlen (result); nth1++) if (result[nth1] != '/') break;
			
			// Phase 3: Two or more slashes
			if (nth1 - nth >= 2) {
				char *address1 = libtext_cut (result, nth + 1, nth1 - 1, 'd'); if (address1 == NULL) {free (result); return NULL;}
				free (result); result = strdup (address1); free (address1); if (result == NULL) return NULL; nth = -1;
			}
			
			else if ((nth + 1 < strlen (result)) && (result[nth + 1] == '.')) {
				// Phase 4: Stalemate
				if ((nth + 1 == strlen (result) - 1) || (result[nth + 2] == '/')) {
					char *address1 = libtext_cut (result, nth, nth + 1, 'd'); if (address1 == NULL) {free (result); return NULL;}
					free (result); result = strdup (address1); free (address1); if (result == NULL) return NULL; nth = -1;
				}
				
				// Phase 5: Step backward
				else if ((nth + 2 < strlen (result)) && (result[nth + 2] == '.') && ((nth + 2 == strlen (result) - 1) || (result[nth + 3] == '/'))) {
					if (nth == 0) {
						if (nth + 2 == strlen (result) - 1) {free (result); return strdup ("/");}
						else {
							char *address1 = libtext_cut (result, 0, 2, 'd'); if (address1 == NULL) {free (result); return NULL;}
							free (result); result = strdup (address1); free (address1); if (result == NULL) return NULL; nth = -1;
						}
					}
					else {
						for (nth1 = nth - 1; nth1 >= 0; nth1--) if ((result[nth1] == '/') && (libtext_escaped (result, nth1) == 'n')) break; if (nth1 >= 0) {
							char *address1 = nth1 <= 0 ? strdup ("/") : libtext_cut (result, nth1, nth + 2, 'd'); if (address1 == NULL) {free (result); return NULL;}
							free (result); result = strdup (address1); free (address1); if (result == NULL) return NULL; nth = -1;
						}
					}
				}
			}
		}
		
		nth++;
	}
	
	return result;
}

char *libfile_parent (const char *address) {
	char *address1 = libtext_connect (2, address, "/.."); if (address1 == NULL) return NULL;
	char *address2 = libfile_refine_address (address1); free (address1); return address2;
}

char *libfile_name (const char *address) {
	if (address == NULL) return NULL;
	char *address1 = libfile_refine_address (address); if (address1 == NULL) return NULL;
	if (strcmp (address1, "/") == 0) return address1;
	int nth; for (nth = strlen (address1) - 1; nth >= 0; nth--) if ((address[nth] == '/') && (libtext_escaped (address, nth) == 'n')) break;
	char *address2 = libtext_cut (address1, nth + 1, strlen (address1) - 1, 't'); free (address1); return address2;
}

char *libfile_status (const char *address, char category) {
	if (address == NULL) return NULL;
	struct stat status; if (lstat (address, &status) == -1) return NULL;
	if (category == 't') {
		if (S_ISREG (status.st_mode)) return strdup ("Regular file");
		else if (S_ISDIR (status.st_mode)) return strdup ("Directory");
		else if (S_ISCHR (status.st_mode)) return strdup ("Character device");
		else if (S_ISBLK (status.st_mode)) return strdup ("Block device");
		else if (S_ISFIFO (status.st_mode)) return strdup ("FIFO");
		else if (S_ISLNK (status.st_mode)) return strdup ("Symbolic link");
		else if (S_ISSOCK (status.st_mode)) return strdup ("Socket");
		else return strdup ("Unknown type");
	}
	else if (category == 's') return libtext_number ((int) status.st_size);
	else if (category == 'o') {
		struct passwd *owner = getpwuid (status.st_uid); if (owner == NULL) return NULL;
		return strdup (owner->pw_name);
	}
	else if (category == 'g') {
		struct group *group = getgrgid (status.st_gid); if (group == NULL) return NULL;
		return strdup (group->gr_name);
	}
}

char libfile_create (const char *address, char type, char with_parents) {
	if ((address == NULL) || ((type != 'f') && (type != 'd'))) return 'f';
	char *address1 = libfile_refine_address (address); if (address1 == NULL) return 'f';
	int gate; if (type == 'd') gate = mkdir (address1, 0700); else gate = open (address1, O_RDWR | O_CREAT, 0600);
	if (gate != -1) {if (type == 'f') close (gate); free (address1); return 's';}
	else if (errno == EEXIST) {
		char result1 = 'f';
		if (type == 'f') {gate = open (address1, O_RDONLY); if (gate != -1) {result1 = 's'; close (gate);}}
		else if (type == 'd') {DIR *gate1 = opendir (address1); if (gate1 != NULL) {result1 = 's'; closedir (gate1);}}
		free (address1); return result1;
	}
	else if (errno == ENOENT) {
		if (with_parents == 'y') {
			char *address2 = libfile_parent (address1); free (address1); if (address2 == NULL) return 'f';
			if (libfile_create (address2, 'd', 'y') != 's') {free (address2); return 'f';} free (address2);
			return libfile_create (address, type, with_parents);
		}
		else {free (address1); return 'f';}
	}
	else {free (address1); return 'f';}
}

char *libfile_read (const char *address, int from, int to) {
	if (address == NULL) return NULL;
	struct stat status; lstat (address, &status); //if (status == NULL) return NULL;
	int from1 = from; if (from1 < -1) from1 = 0; else if ((from == -1) || (from1 >= status.st_size)) from1 = status.st_size - 1;
	int to1 = to; if ((to == -1) || (to >= status.st_size)) to1 = status.st_size - 1; else if (to < -1) to1 = 0;
	if (from1 > to1) return NULL;

	int file = open (address, O_RDONLY); if (file == -1) return NULL;
	char *content = (char *) malloc ((to1 - from1) + 1 + 1); if (content == NULL) {close (file); return NULL;}

	pread (file, content, to1 - from1, from1);
	content[(to1 - from1) + 1] = '\0';

	close (file);
	return content;
}

char libfile_write (const char *address, const char *content, char type, int where, char create, char *brake) {
	if ((address == NULL) || (content == NULL)) return 'f';
	char *address1 = libfile_refine_address (address); if (address1 == NULL) return 'f';
	int file; if (create == 'n') file = open (address1, O_RDONLY); else file = open (address1, O_RDWR | O_CREAT, 0600); if (file == -1) {free (address1); return 'f';}
	int end = lseek (file, 0, SEEK_END); if (where > end) where = end; else if (where < 0) where = 0;
	lseek (file, where, SEEK_SET);
	
	if (type == 'f') {
		int file1 = open (content, O_RDONLY); if (file1 == -1) {close (file); free (address1); return 'f';}
		char data; ssize_t result = sizeof (char); while (result >= sizeof (char)) {
			if ((brake != NULL) && (*brake != 0)) {close (file1); close (file); free (address1); return 'f';}
			result = read (file1, &data, sizeof (char)); if (result == -1) {close (file1); close (file); free (address1); return 'f';}
			write (file, &data, result);
		}
		close (file1); close (file); free (address1); return (result != -1) ? 's' : 'f';
	}
	else {
		int result = write (file, (void *) content, strlen (content));
		close (file); free (address1); return result == strlen (content) ? 's' : 'f';
	}
}

char libfile_copy (const char *from, const char *to, char *brake) {
	if ((from == NULL) || (to == NULL)) return 'f';
	if (access (from, F_OK | R_OK) != 0) return 'f';
	char *type = libfile_status (from, 't'); if (type == NULL) return 'f'; int result = strcmp (type, "Directory"); free (type);
	if (result == 0) {
		if ((brake != NULL) && (*brake != 0)) return 'f';
		if (libfile_create (to, 'd', 'y') != 's') return 'f';
		DIR *from2 = opendir (from); if (from2 == NULL) return 'f';
		struct dirent *from3; while ((from3 = readdir (from2)) != NULL) {
			if ((brake != NULL) && (*brake != 0)) {closedir (from2); return 'f';}
			if ((strcmp (from3->d_name, ".") == 0) || (strcmp (from3->d_name, "..") == 0)) continue;
			if (access (from, F_OK | R_OK) != 0) {closedir (from2); return 'f';}
			char *name = strdup (from3->d_name); if (name == NULL) {closedir (from2); return 'f';}
			char *from4 = libtext_connect (3, from, "/", name); if (from4 == NULL) {free (name); closedir (from2); return 'f';}
			char *to2 = libtext_connect (3, to, "/", name); if (to2 == NULL) {free (from4); free (name); closedir (from2); return 'f';}
			if ((brake != NULL) && (*brake != 0)) {free (to2); free (from4); free (name); closedir (from2); return 'f';}
			char result1 = libfile_copy (from4, to2, brake); free (to2); free (from4); free (name); if (result1 != 's') {closedir (from2); return 'f';} 
		}
		closedir (from2); return 's';
	}
	else {
		if ((brake != NULL) && (*brake != 0)) return 'f';
		char result = libfile_write (to, from, 'f', 0, 'y', brake); return result;
	}
}

char libfile_erase (const char *address, char *brake) {
	if (address == NULL) return 'f';
	char *address0 = libfile_refine_address (address); if (address0 == NULL) return 'f';
	if (access (address0, F_OK | R_OK | W_OK) != 0) {free (address0); return 'f';}
	char *type = libfile_status (address0, 't'); if (type == NULL) {free (address0); return 'f';} int result = strcmp (type, "Directory"); free (type);
	if (result == 0) {
		if ((brake != NULL) && (*brake != 0)) {free (address0); return 'f';}
		
		DIR *directory = opendir (address0); if (directory == NULL) {free (address0); return 'f';}
		struct dirent *directory1; while ((directory1 = readdir (directory)) != NULL) {
			if ((brake != NULL) && (*brake != 0)) {closedir (directory); free (address0); return 'f';}
			if ((strcmp (directory1->d_name, ".") == 0) || (strcmp (directory1->d_name, "..") == 0)) continue;
			if (access (address0, F_OK | R_OK | W_OK) != 0) {closedir (directory); free (address0); return 'f';}
			char *name = strdup (directory1->d_name); if (name == NULL) {closedir (directory); free (address0); return 'f';}
			char *address1 = libtext_connect (3, address0, "/", name); if (address1 == NULL) {free (name); closedir (directory); free (address0); return 'f';}
			if ((brake != NULL) && (*brake != 0)) {free (address1); free (name); closedir (directory); free (address0); return 'f';}
			char result1 = libfile_erase (address1, brake);
			free (address1); free (name); if (result1 != 's') {closedir (directory); free (address0); return 'f';}
		}
		closedir (directory); result = rmdir (address0); free (address0); return result == 0 ? 's' : 'f';
	}
	else {result = unlink (address0); free (address0); return result == 0 ? 's' : 'f';}
}

char libfile_move (const char *from, const char *to, char *brake) {
	if ((from == NULL) || (to == NULL)) return 'f';
	if ((brake != NULL) && (*brake != 0)) return 'f';
	if (rename (from, to) == 0) return 's';
	if (errno != EXDEV) return 'f';
	char result = libfile_copy (from, to, brake); if (result != 's') return 'f';
	if ((brake != NULL) && (*brake != 0)) return 'f';
	return libfile_erase (from, brake);
}

#!/bin/bash

if (test "$1" = "build"); then
	rm -rf libfile.so || exit 1
	gcc libfile.c $(pkg-config --cflags --libs libtext) -lm -shared -o libfile.so || exit 1
	
elif (test "$1" = "install"); then
	cp -f libfile.so /usr/lib || exit 1
	cp -f libfile.h /usr/include || exit 1
	cp -f libfile.pc /usr/lib/pkgconfig || exit 1
	
elif (test "$1" = "remove"); then
	rm -rf /usr/lib/libfile.so /usr/include/libfile.h /usr/lib/pkgconfig/libfile.pc || exit 1
	
fi

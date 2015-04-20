# Makefile for libhuff

include ../Makefile.cfg

CC = psx-gcc
AR = mipsel-unknown-elf-ar
RANLIB = mipsel-unknown-elf-ranlib

all: libhuff.a

huff.o: huff.c
	$(CC) -c huff.c

libhuff.a: huff.o
	rm -f libhuff.a
	$(AR) r libhuff.a huff.o
	$(RANLIB) libhuff.a	

install: all
	cp libhuff.a $(TOOLCHAIN_PREFIX)/lib
	cp huff.h $(TOOLCHAIN_PREFIX)/include

clean:
	rm -f *.o *.a
# Makefile for PSX SDK library

include ../Makefile.cfg

CC = mipsel-unknown-elf-gcc
CFLAGS = -Iinclude/ -G0 -O0 -fno-builtin -mno-gpopt -nostdlib -msoft-float
AS = mipsel-unknown-elf-as
LD = mipsel-unknown-elf-ld
AR = mipsel-unknown-elf-ar
RANLIB = mipsel-unknown-elf-ranlib

all: libpsx.a

libpsx.a: out/syscalls.o out/psxsdk.o out/libc.o out/setup.o out/memory.o out/gpu.o out/memcard.o start.o
	$(AR) r libpsx.a out/*
	$(RANLIB) libpsx.a

out/syscalls.o: src/syscalls.s
	$(CC) $(CFLAGS) -c -o out/syscalls.o src/syscalls.s

out/psxsdk.o: src/psxsdk.c
	$(CC) $(CFLAGS) -c -o out/psxsdk.o src/psxsdk.c
	
out/libc.o: src/libc.c
	$(CC) $(CFLAGS) -c -o out/libc.o src/libc.c

out/setup.o: src/setup.c
	$(CC) $(CFLAGS) -c -o out/setup.o src/setup.c

out/memory.o: src/memory.c
	$(CC) $(CFLAGS) -c -o out/memory.o src/memory.c

out/gpu.o: src/gpu.c
	$(CC) $(CFLAGS) -c -o out/gpu.o src/gpu.c

out/memcard.o: src/memcard.c
	$(CC) $(CFLAGS) -c -o out/memcard.o src/memcard.c

start.o: src/start.s
	$(CC) $(CFLAGS) -c -o start.o src/start.s

install:
	cp libpsx.a $(TOOLCHAIN_PREFIX)/lib
	cp start.o $(TOOLCHAIN_PREFIX)/lib
	cp -r include/*	$(TOOLCHAIN_PREFIX)/include
	
clean:
	rm -f out/*
	rm -f libpsx.a
	rm -f start.o

distclean: clean

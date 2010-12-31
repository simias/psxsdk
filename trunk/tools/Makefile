# Makefile for the PSXSDK tools

include ../Makefile.cfg

CC = gcc
CFLAGS = -Wall -g
TOOL_LIST = bmp2tim elf2exe getpsxiso mkpsxiso vag2wav wav2vag exefixup \
		systemcnf bin2c huff mod4psx tim2bmp lictool

all: $(TOOL_LIST)

bmp2tim: bmp2tim.c
	$(CC) $(CFLAGS) -o bmp2tim bmp2tim.c

elf2exe: elf2exe.c
	$(CC) $(CFLAGS) -o elf2exe elf2exe.c

getpsxiso: getpsxiso.c
	$(CC) $(CFLAGS) -o getpsxiso getpsxiso.c

mkpsxiso: mkpsxiso.c
	$(CC) $(CFLAGS) -o mkpsxiso mkpsxiso.c
	
vag2wav: vag2wav.c
	$(CC) $(CFLAGS) -o vag2wav vag2wav.c
	
wav2vag: wav2vag.c
	$(CC) $(CFLAGS) -o wav2vag wav2vag.c

exefixup: exefixup.c
	$(CC) $(CFLAGS) -o exefixup exefixup.c
	
systemcnf: systemcnf.c
	$(CC) $(CFLAGS) -o systemcnf systemcnf.c

bin2c: bin2c.c
	$(CC) $(CFLAGS) -o bin2c bin2c.c

huff: huff.c
	$(CC) $(CFLAGS) -o huff huff.c

mod4psx: mod4psx.c adpcm.c
	$(CC) $(CFLAGS) -o mod4psx mod4psx.c adpcm.c ../libmodplay/libmodplay_nopsx.a -lm

tim2bmp: tim2bmp.c
	$(CC) $(CFLAGS) -o tim2bmp tim2bmp.c -lz
	
lictool: lictool.c
	$(CC) $(CFLAGS) -o lictool lictool.c

clean:
	rm -f $(TOOL_LIST)

distclean: clean

install:
	cp -rv $(TOOL_LIST) $(TOOLCHAIN_PREFIX)/bin

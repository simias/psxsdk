include ../../Makefile.cfg

CFLAGS = 

all:
	psx-gcc -DEXAMPLES_VMODE=$(EXAMPLES_VMODE) -o psxpaint.elf psxpaint.c
	elf2exe psxpaint.elf psxpaint.exe
	cp psxpaint.exe cd_root
	systemcnf psxpaint.exe > cd_root/system.cnf
	mkisofs -o psxpaint.hsf -V PSXPAINT -sysid PLAYSTATION cd_root
	mkpsxiso psxpaint.hsf psxpaint.bin $(TOOLCHAIN_PREFIX)/share/licenses/infousa.dat
	rm -f psxpaint.hsf
	
clean:
	rm -f psxpaint.bin psxpaint.cue psxpaint.exe psxpaint.elf
	rm -fr cd_root/*

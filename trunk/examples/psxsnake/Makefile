include ../../Makefile.cfg

CFLAGS = 

all: psxsnake.exe build_data make_image

psxsnake.exe: psxsnake.c
	psx-gcc -DEXAMPLES_VMODE=$(EXAMPLES_VMODE) \
	$(CFLAGS) -o psxsnake.elf psxsnake.c
	elf2exe psxsnake.elf psxsnake.exe

build_data:
	$(MAKE_COMMAND) -C data

clean:
	rm -f psxsnake.elf psxsnake.exe cdimg/*
	$(MAKE_COMMAND) -C data clean

# This target first makes a CD root filesystem, then an ISO image
# and then makes a licensed .bin/.cue image ready to be burned for a
# modded PlayStation

make_image:
	mkdir -p cd_root
	systemcnf psxsnake.exe > cd_root/system.cnf
	cp psxsnake.exe cd_root
	cp data/*.tim cd_root
	cp data/*.raw cd_root
	mkisofs -o psxsnake.hsf -V PSXSNAKE -sysid PLAYSTATION cd_root
	mkpsxiso psxsnake.hsf psxsnake.bin $(TOOLCHAIN_PREFIX)/share/licenses/infousa.dat
	mv psxsnake.bin psxsnake.cue cdimg
	rm -f psxsnake.hsf
	rm -fr cd_root

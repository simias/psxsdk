include ../../Makefile.cfg

CFLAGS = 

all: vagtest.exe build_data make_image

vagtest.exe: vagtest.c
	psx-gcc -DEXAMPLES_VMODE=$(EXAMPLES_VMODE) \
	$(CFLAGS) -o vagtest.elf vagtest.c
	elf2exe vagtest.elf vagtest.exe

build_data:
	$(MAKE_COMMAND) -C data

clean:
	rm -f vagtest.elf vagtest.exe cdimg/*
	$(MAKE_COMMAND) -C data clean

# This target first makes a CD root filesystem, then an ISO image
# and then makes a licensed .bin/.cue image ready to be burned for a
# modded PlayStation

make_image:
	mkdir -p cd_root
	systemcnf vagtest.exe > cd_root/system.cnf
	cp  vagtest.exe cd_root
	mkisofs -o vagtest.hsf -V VAGTEST -sysid PLAYSTATION cd_root
	mkpsxiso vagtest.hsf vagtest.bin $(TOOLCHAIN_PREFIX)/share/licenses/infousa.dat
	mv vagtest.bin vagtest.cue cdimg
	rm -f vagtest.hsf
	rm -fr cd_root

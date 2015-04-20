include ../Makefile.cfg

all:
	sh genscripts.sh $(TOOLCHAIN_PREFIX)

clean:
	rm -f playstation.x psx-gcc

distclean: clean

install:
	mkdir -p $(TOOLCHAIN_PREFIX)/bin
	mkdir -p $(TOOLCHAIN_PREFIX)/mipsel-unknown-elf/lib/ldscripts
	cp -v psx-gcc $(TOOLCHAIN_PREFIX)/bin
	cp -v playstation.x $(TOOLCHAIN_PREFIX)/mipsel-unknown-elf/lib/ldscripts

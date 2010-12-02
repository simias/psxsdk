# Makefile to build PSXSDK

include Makefile.cfg

build:
	$(MAKE_COMMAND) -C misc
	$(MAKE_COMMAND) -C misc install
	sh -c "export PATH=$$PATH:$(TOOLCHAIN_PREFIX)/bin;$(MAKE_COMMAND) -C libpsx"
	$(MAKE_COMMAND) -C libpsx install
	sh -c "export PATH=$$PATH:$(TOOLCHAIN_PREFIX)/bin;$(MAKE_COMMAND) -C libhuff" 
	$(MAKE_COMMAND) -C libhuff install
	sh -c "export PATH=$$PATH:$(TOOLCHAIN_PREFIX)/bin;$(MAKE_COMMAND) -C libm" 
	$(MAKE_COMMAND) -C libm install
	sh -c "export PATH=$$PATH:$(TOOLCHAIN_PREFIX)/bin;$(MAKE_COMMAND) -C libadpcm"
	$(MAKE_COMMAND) -C libadpcm install
	sh -c "export PATH=$$PATH:$(TOOLCHAIN_PREFIX)/bin;$(MAKE_COMMAND) -C libmodplay"
	$(MAKE_COMMAND) -C libmodplay install
	$(MAKE_COMMAND) -C tools
	$(MAKE_COMMAND) -C tools install
	$(MAKE_COMMAND) -C licenses install

clean:
	$(MAKE_COMMAND) -C libpsx clean
	$(MAKE_COMMAND) -C libhuff clean
	$(MAKE_COMMAND) -C libm clean
	$(MAKE_COMMAND) -C libadpcm clean
	$(MAKE_COMMAND) -C libmodplay clean
	$(MAKE_COMMAND) -C misc clean
	$(MAKE_COMMAND) -C tools clean

distclean:
	$(MAKE_COMMAND) -C libpsx distclean
	$(MAKE_COMMAND) -C libhuff clean
	$(MAKE_COMMAND) -C libm clean
	$(MAKE_COMMAND) -C libadpcm clean
	$(MAKE_COMMAND) -C libmodplay clean
	$(MAKE_COMMAND) -C misc distclean
	$(MAKE_COMMAND) -C tools distclean

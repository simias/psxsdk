#!/bin/sh

# This shell script will generate the playstation.x linker script, and the psx-gcc and psx-elf2x shell scripts.

# You have to pass the PREFIX of the toolchain as the first argument of this shell script

echo "/* 
 * Linker script to generate an ELF file
 * that has to be converted to PS-X EXE.
 */

TARGET(\"elf32-littlemips\")
OUTPUT_ARCH(\"mips\")

ENTRY(\"_start\")

SEARCH_DIR(\"$1/lib\")
SEARCH_DIR(\"$1/include\")
STARTUP(start.o)
INPUT(-lpsx)

EXTERN(__udivdi3)
EXTERN(__truncdfsf2)
EXTERN(__floatdidf)
EXTERN(__floatsidf)
EXTERN(__divdf3)
EXTERN(__adddf3)
EXTERN(__muldf3)
EXTERN(__fixdfsi)

SECTIONS
{
	. = 0x80010000;
	__text_start = .;
	.text : { *(.text) }
	__text_end = .;
	__data_start = .; 
	.data ALIGN(4) : { *(.data) }
	__date_end = .;
	__bss_start = .;
	.bss  ALIGN(4) : { *(.bss) }
	__bss_end = .;
}
" > playstation.x

echo "#!/bin/sh
mipsel-unknown-elf-gcc -fsigned-char -msoft-float -mno-gpopt -fno-builtin -G0 -O0 -I$1/include -T $1/mipsel-unknown-elf/lib/ldscripts/playstation.x \$*"> psx-gcc
chmod +x psx-gcc


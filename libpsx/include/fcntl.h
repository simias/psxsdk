/*
 * fcntl.h
 *
 * File control
 *
 * PSXSDK
 */

#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY	1
#define O_WRONLY	2
#define	O_APPEND	8
#define O_RDWR		(O_RDONLY | O_WRONLY)
#define O_CREAT		512
#define O_TRUNC		1024

/*
 * These are standard C library file I/O functions provided by the PSX BIOS.
 * Filenames have to be specified in this way:
 * <device>:<filename>
 *
 * Where device specifies the device the file is on:
 * "tty:"		Console
 * "cdrom:"		CD-ROM
 * "buXX:"		Memory cards
 *
 * When using cdrom: as device, append file version (;1) to filename
 * Example: cdrom:README.TXT;1
 *
 * Subdirectory paths have to be specified with backslashes (\),
 * like MS-DOS. Read and write operations can be carried only in blocks. 
 * Blocks are 2048 bytes for the CD-ROM device, and 128 bytes for memory cards.
 */

int open(char *filename, int mode);

#endif


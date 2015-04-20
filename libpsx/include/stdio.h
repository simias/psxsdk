/*
 * stdio.h implementation for PSXSDK
 */

#ifndef _STDIO_H
#define _STDIO_H

#ifdef _PSXSDK_WRAPPER

/*
 * Dirty hack... 
 */

#include "/usr/include/stdio.h"

#else

#include <stdarg.h>
#include <stdbool.h>

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define EOF		-1

/* NULL */
#ifndef NULL
#define NULL (void*)0
#endif

/*
 * FILE structure.
 */

typedef struct
{
	int fildes;
	unsigned int pos;
	unsigned int mode;
	unsigned int dev;
	unsigned int size;
	unsigned int used;
}FILE;

/*
 * The functions below are just prototypes for assembly wrappers which
 * call BIOS functions, so they're actually supplied by the BIOS
 * and not implemented by the PSX SDK.
 */

/* Console functions */

extern int putchar(int c);
extern int puts(const char *str);

/*
 * BIOS printf() implementation. Does not support floating point.
 */

extern int printf(char *format, ...);

int vsnprintf(char *string, unsigned int size, char *fmt, va_list ap);
int vsprintf(char *string, char *fmt, va_list ap);
int sprintf(char *string, char *fmt, ...);
int snprintf(char *string, unsigned int size, char *fmt, ...);

FILE *fdopen(int fildes, const char *mode);
FILE *fopen(char *path, const char *mode);
int fclose(FILE *stream);
int fread(void *ptr, int size, int nmemb, FILE *f);

int fgetc(FILE *f);
int ftell(FILE *f);
int fseek(FILE *f, int offset, int whence);

#define getc(f)		fgetc(f)

int rename(char *oldname, char *newname);
int delete(char *filename);

#define remove(x)	delete(x)

#endif

#endif


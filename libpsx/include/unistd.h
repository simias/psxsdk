/*
 * unistd.h
 *
 * PSXSDK
 */

#ifndef _UNISTD_H
#define _UNISTD_H

int lseek(int fd, int off, int whence);
int read(int fd, void *buf, int nbytes);
int write(int fd, void *buf, int nbytes);
int close(int fd);
int cd(char *dirname);

#define chdir(x)	cd(x)

#endif


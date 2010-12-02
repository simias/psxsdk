/*
 * string.h
 *
 * Prototypes for string functions of the C library
 *
 * PSXSDK
 */

// Some of these functions are actually wrappers to those supplied by the 
// PlayStation BIOS. Check the source code of PSXSDK for more details.

#ifndef _STRING_H
#define _STRING_H

char *strcat(char *dst , char *src);
char *strncat(char *dst , char *src , int n);
int strcmp(char *dst , char *src);
int strncmp(char *dst , char *src , int n);
char *strcpy(char *dst , char *src);
char *strncpy(char *dst , char *src , int n);
int strlen(char *s);
int index(char *s , int c);
int rindex(char *s , int c);
char *strchr(char *s , int c);
char *strrchr(char *s , int c);
char *strpbrk(char *dst , char *src);
int strspn(char *s , char *set);
int strcspn(char *s , char *set);
char *strtok(char *s , char *set);
char *strstr(char *big , char *little);

void *memset(void *dst , char c , int n);
void *memmove(void *dst , void *src , int n);
int memcmp(void *b1 , void *b2 , int n);
void *memchr(void *s , int c , int n);
void *memcpy(void *dst , void *src , int n);

#endif


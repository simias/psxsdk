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

char *strcat(char *s , const char *append);
char *strncat(char *s , const char *append, int n);
int strcmp(const char *dst , const char *src);
int strncmp(const char *dst , const char *src , int len);
char *strcpy(char *dst , const char *src);
char *strncpy(char *dst , const char *src , int n);
int strlen(const char *s);
int index(char *s , int c);
int rindex(char *s , int c);
char *strchr(const char *s , int c);
char *strrchr(const char *s , int c);
char *strpbrk(const char *dst , const char *src);
int strspn(char *s , char *set);
int strcspn(char *s , char *set);
char *strtok(char *s , char *set);
char *strstr(const char *big , const char *little);

void *memset(void *dst , char c , int n);
void *memmove(void *dst , const void *src , int n);
int memcmp(const void *b1 , const void *b2 , int n);
void *memchr(void *s , int c , int n);
void *memcpy(void *dst , const void *src , int n);

#endif


/*
 * stdlib.h
 *
 * Standard library functions
 *
 * PSXSDK
 */

#ifndef _STDLIB_H
#define _STDLIB_H

/* Conversion functions */

extern int atoi(char *s);
extern long atol(char *s);
extern char atob(char *s); // Is this right?

// Random number functions

int rand();
void srand(unsigned int seed);

// Quick sort

void qsort(void *base , int nel , int width , int (*cmp)(const void *,const void *));

// Memory allocation functions

#warning "malloc() family of functions NEEDS MORE TESTING"

void *malloc(int size);
void free(void *buf);
void *calloc(int number, int size);
void *realloc(void *buf , int n);

int abs(int j);
long long strtoll(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);
double strtod(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);
float strtof(const char *nptr, char **endptr);

#endif


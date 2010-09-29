/* 
 * search.h
 *
 * PSXSDK
 */

#ifndef _SEARCH_H
#define _SEARCH_H

void *lsearch(void *key , void *base , int belp , int width , int (*cmp)(void * , void *));
void *bsearch(void *key , void *base , int nel , int size , int (*cmp)(void * , void *));

#endif


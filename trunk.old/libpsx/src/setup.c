#include <psx.h>
#include "memory.h"

extern int __bss_start[];
extern int __bss_end[];

void psxsdk_setup()
{
	unsigned int x;

	printf("Initializing PSXSDK... \n");

// Clear BSS space	
	for(x = (unsigned int)__bss_start; x < (unsigned int)__bss_end; x++)
		*((unsigned char*)x) = 0;

// Setup memory allocation functions
	malloc_setup();

}

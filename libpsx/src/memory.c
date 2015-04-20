/* 
 * memory.c
 *
 * PSXSDK malloc() family functions
 */

#include <stdio.h>
#include <stdlib.h>

extern int __bss_start[];
extern int __bss_end[];

unsigned int first_free_page;

// 1K granularity and "pages", so more allocations which are small can be done

unsigned char busy_pages[2048];
unsigned int  alloc_size[2048];

// RAM memory map on the PSX
// 0x80000000 - 0x8000FFFF		RAM used by the BIOS
// 0x80010000 - 0x801FFFFF		Program memory

void malloc_setup()
{
	int x;

	printf("malloc setup.\n");

	first_free_page = (unsigned int) __bss_end;
	first_free_page-= 0x80000000;
	
	if(first_free_page & 0x3ff)
		first_free_page = (first_free_page | 0x3ff) + 1;

	first_free_page>>=10;

	//printf("First free page: %d, bss_end: %x\n", first_free_page, __bss_end);
	
	for(x = 0; x < first_free_page; x++)
	{
		busy_pages[x] = 1; // RAM occupied by program, data and BIOS looks always allocated
		alloc_size[x] = 1; // Fake that 1K was required
	}
		
	for(x = first_free_page; x < 2048; x++)
	{
		busy_pages[x] = 0;
		alloc_size[x] = 0;
	}
}

void *malloc(int size)
{
	int x, y, z;
	
// Round size

	if(size & 0x3ff)
		size = (size | 0x3ff) + 1;
		
// Divide it by 1024	
	size >>= 10;
	//printf("Allocating %dKb\n", size);
	
// Find a free page
	for(x = 0; x < 2048; x++)
	{
		if(busy_pages[x] == 0)
		{
			// If we find a free page, check how many free pages follow it.
			// If it's enough for the memory we want to allocate, then return
			// the pointer to the free page we found, otherwise keep finding
			
		//	printf("Page found at %dKb\n", x);
			
			for(y = 0; y < size; y++)
				if(busy_pages[x+y] == 1) goto malloc_keep_finding;
				
			// We found the memory we wanted, now make it busy
			
			for(y = 0; y < size; y++)
				busy_pages[x+y] = 1;
			
			// Store allocation size, it is essential for free()	
						
			alloc_size[x] = size;
		//	printf("malloc(): alloc_size[%d] = %d\n", x, size);
				
			return (void*)((unsigned int)0x80000000 + (x<<10));
		}
malloc_keep_finding:
		; // Useless statement to make GCC not bail out...
	}
	
// We couldn't find anything, return NULL
	return NULL;
}

void *calloc(int number, int size)
{
	void *ptr = malloc(number * size);
	unsigned char *cptr = (unsigned char*)ptr;
	int x;
	
	if(ptr == NULL)
		ptr = NULL;
		
	for(x = 0; x < (number * size); x++)
		cptr[x] = 0;
		
	return ptr;
}																

void free(void *ptr)
{
	unsigned int ptri = (unsigned int)ptr;
	ptri -= 0x80000000;
	int x;
	
	if((ptri & 0x3ff) || (busy_pages[ptri>>10] == 0) || (alloc_size[ptri>>10] == 0))
	{
		// If the pointer address is not a multiplier of 1K, or the page
		// is free, it means that memory not allocated by malloc() was passed to free.
		// Print a warning message and return.
		
		printf("** free() ** : tried to free memory with invalid pointer at %x\n",
			ptri + 0x80000000);
			
		return;	
	}
	
// Divide ptri by 1024, getting initial page
	
	ptri>>=10;

//	printf("Freeing page at %dKb\n", ptri);

// Free pages

//	printf("alloc_size[%d] = %d\n", ptri, alloc_size[ptri]);

	for(x = 0; x < alloc_size[ptri]; x++)
	{
		printf("ptri + x = %d\n", ptri + x);
		busy_pages[ptri + x] = 0;
	}

// Set allocation size to 0, finally freeing initial page		
						
	alloc_size[ptri] = 0;
	
	/*for(x=150;x<170;x++)
		printf("%d: %d, %d\n", x, busy_pages[x], alloc_size[x]);*/
}
																																															
#include <stdio.h>
#include <stdlib.h>
#include "../libmodplay/modplay.h"
#include "adpcm.h"

unsigned char *mod_data;
ModMusic *mod;

// Container format

// Header

// 8 bytes - "_mod4psx"
// 4 bytes - Number of samples contained

// Sample format
// 4 bytes - Length of ADPCM sample
// 8 bytes - Reserved
// ... Data ...

// All multi word numerical values are in little endian format
// which is used by the processor of the PlayStation.
// All data is aligned to 4 bytes.

unsigned char adpcm_buffer[0x10000];

int main(int argc, char *argv[])
{
	FILE *f;
	int sz, x,y;
	
	if(argc < 3)
	{
		printf("mod4psx <mod_music> <adpcm_dat>\n");
		printf(
"\nMOD4PSX gets the sound samples from a music module supported by libmodplay, "
"and then converts them to PS1 ADPCM format and puts them all in a datafile, which will be able to be loaded "
"by libmodplay. In this way the CPU time needed by the PlayStation processor to convert at runtime from PCM to ADPCM is saved.\n"
);
		return -1;
	}
	
	f = fopen(argv[1], "rb");
	
	if(f == NULL)
	{
		printf("Could not open %s for reading. Aborting.\n", argv[1]);
		return -1;
	}
	
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	mod_data = malloc(sz);
	
	if(mod_data == NULL)
	{
		printf("Could not allocate %d bytes of memory. Aborting.\n", sz);
		return -1;
	}
	
	fread(mod_data, sizeof(char), sz, f);
	
	fclose(f);
	
	mod = MODLoad(mod_data);
	
	printf("Title: %s\n", mod->title);
	

	
	f = fopen(argv[2], "wb");
	
// Write header	
	
// Magic string	
	fprintf(f, "_mod4psx");
// Write number of samples	
	fputc(mod->sample_num & 0xff, f);
	fputc((mod->sample_num >> 8) & 0xff, f);
	fputc(0, f);
	fputc(0, f);
	
	for(x = 0; x < mod->sample_num; x++)
	{
		
		if(mod->sample[x].length != 0)
		{
			if(mod->fmt == MOD_FMT_MOD &&
			    mod->sample[x].repeat_len > 2)
			{
				
			sz = SsAdpcmPack(mod->sample[x].data, adpcm_buffer,
				mod->sample[x].length, FMT_U8, sizeof(adpcm_buffer),
				1, mod->sample[x].repeat_off);
				
			}
			else
			{
				
			sz = SsAdpcmPack(mod->sample[x].data, adpcm_buffer,
				mod->sample[x].length, FMT_U8, sizeof(adpcm_buffer),
				0, 0);
				
			}

			printf("%d) %s, %d -> %d, %d, %d, FIN=%d\n", x, mod->sample[x].name,
				mod->sample[x].length, sz,
				mod->sample[x].repeat_off,
					mod->sample[x].repeat_len, mod->sample[x].finetune);
		}
		else
		{
			printf("%d) Not written\n", x);
			sz = 0;
		}
			
// Write length of ADPCM sample		
		
		fputc(sz & 0xff, f);
		fputc((sz>>8)&0xff, f);
		fputc((sz>>16)&0xff, f);
		fputc((sz>>24)&0xff, f);

// Skip 8 reserved bytes - for future expansion...
		
		fseek(f, 8, SEEK_CUR);

// Write ADPCM sample data
		
// Manipulate the samples to do looping for Protracker MOD samples
// The PCM to ADPCM conversion routines haven't been modified to do this yet.		
// This won't be done for Composer 669 module files for now.
		
		/*if(mod->sample[x].repeat_len > 2 &&
			mod->fmt == MOD_FMT_MOD)
		{
		
			for(y = 0; y < ((sz / 16)-1); y++)
			{
				if((mod->sample[x].repeat_off / 28) == y)
					adpcm_buffer[(y<<4) + 1] = 6;
				else	
					adpcm_buffer[(y<<4) + 1] = 2;
			}
		
			adpcm_buffer[(y<<4) + 1] = 3;
		}*/
			
		fwrite(adpcm_buffer, sizeof(char), sz, f);
	}
	
	return 0;
}

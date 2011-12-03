/*
 * psfex - Extracts an EXE from a PSF file
 *
 * Programmed by Giuseppe Gatta - released in the public domain
 * It can be used for any platform which the PSF format supports, not only PS1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

// 0-2:	PSF
// 3:		Version byte (0x01 for PlayStation)
// 4:		Size of reserved area (LE unsigned 32-bit)
// 8:		Compressed program length (LE unsigned 32-bit)
// 12:	Compressed program CRC32 (LE unsigned 32-bit)
// xxx:	Reserved area
// xxx:	Compressed program

int main(int argc, char *argv[])
{
	FILE *f;
	unsigned char *fm;
	unsigned int i;
	unsigned int res_size;
	unsigned int cprg_size;
	unsigned int cprg_crc32;
	unsigned char *om;
	unsigned long dest_len;
	int sz;
	
	if(argc < 3)
	{
		printf("psfex - Extracts an executable from a PSF file\n");
		printf("psfex [.psf] [output]\n");
		return -1;
	}
	
	f = fopen(argv[1], "rb");
	
	if (f == NULL)
	{
		printf("Could not open file.\n");
		return -1;
	}

/*
 * Get PSF size and load it in memory 
 */
	
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	fm = malloc(sz);
	fread(fm, sizeof(char), sz, f);
	
	fclose(f);
	
/*
 * Get header information
*/
	if(fm[0] == 'P' && fm[1] == 'S' && fm[2] == 'F')
	{
		printf("PSF file.\n");
	}
	else
	{
		printf("Not a PSF file. Aborting.\n");
		return -1;
	}
	
	res_size = fm[4] | (fm[5] << 8) | (fm[6]<<16) | (fm[7]<<24);
	cprg_size = fm[8] | (fm[9] << 8) | (fm[10]<<16)|(fm[11]<<24);
	cprg_crc32 = fm[12] | (fm[13]<<8)|(fm[14]<<16)|(fm[15]<<24);
	
	printf("Reserved area size: %d bytes\n", res_size);
	printf("Compressed program size: %d bytes\n", cprg_size);
	printf("Compressed program CRC32: 0x%08x\n", cprg_crc32);
	
/*
 * Decompress the program
 * The PSF format is inherently flawed and so we have to allocate 2 megabytes
 * of memory (size of PS1 RAM) and then get the real destination size at the end
 */
	om = malloc(0x200000);
	dest_len = 0x200000;
	uncompress(om, &dest_len, &fm[16 + res_size], cprg_size);
	
	printf("Real destination length: %d\n", dest_len);
	
/*
 * Now let's write the decompressed program to the output file
 */
	f = fopen(argv[2], "wb");
	
	if(f == NULL)
	{
		printf("Could not open %s for writing. Aborting.\n");
		return -1;
	}
	
	fwrite(om, sizeof(char), dest_len, f);
	fclose(f);

/*
 * Free memory, at the exit it is done anyway but this helps adaptions
 */
	
	free(om);
	free(fm);
	
	return 0;
}

/*
 * elf2exe
 *
 * Converts an ELF executable to PS-EXE, using objcopy
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned char psexe_magic[8] = {'P','S','-','X',' ','E','X','E'};
const char *psexe_marker_usa = "Sony Computer Entertainment Inc. for North America area";
const char *psexe_marker_jpn = "Sony Computer Entertainment Inc. for Japan area";
const char *psexe_marker_eur = "Sony Computer Entertainment Inc. for Europe area";
char *psexe_marker;

#define OBJCOPY_PATH		"mipsel-unknown-elf-objcopy"

int main(int argc, char *argv[])
{
	FILE *objcopy_out, *psexe;
	char stringbuf[2048];
	unsigned char charbuf;
	int x;
	unsigned int sz;
	unsigned int gp = 0;
	
	if(argc < 3)
	{
		printf("elf2exe - Converts an ELF executable to PS-EXE\n");
		printf("usage: elf2exe [elf] [ps-x exe] <options>\n");
		printf("\n");
		printf("Options:\n");
		printf("-mark_jpn           - Use Japanese ascii marker (default: USA)\n");
		printf("-mark_eur           - Use European ascii marker (default: USA)\n");
		printf("-mark=<mark>        - Use custom ascii marker <mark>\n");
		return -1;
	}
	
	psexe_marker = (char*)psexe_marker_usa;
	
	for(x = 3; x < argc; x++)
	{
		if(strcmp(argv[x], "-mark_jpn") == 0)
			psexe_marker = (char*)psexe_marker_jpn;
		
		if(strcmp(argv[x], "-mark_eur") == 0)
			psexe_marker = (char*)psexe_marker_eur;
			
		if(strncmp(argv[x], "-mark=", 6) == 0)
		{
			if(strlen(argv[x]) >= 7)
				psexe_marker = argv[x] + 6;
		}
		
		if(strncmp(argv[x], "-gp=", 4) == 0)
		{
			if(strlen(argv[x]) >= 5)
				sscanf(argv[x] + 4, "%x", &gp);
		}	
	}
	
/*
 * Now open the output file
 */
 
	psexe = fopen(argv[2], "wb");
	
	if(psexe == NULL)
	{
		printf("Couldn't open %s for writing. Aborting!\n", argv[2]);
		return -1;
	}

/*
 * Write PSEXE magic string
 */ 
	fwrite(psexe_magic, sizeof(char), 8, psexe);
	
/*
 * Seek output file to 0x10, Initial Program Counter
 */
	fseek(psexe, 0x10, SEEK_SET);

/*
 * Write initial program counter = 0x80010000
 */
	charbuf = 0x00;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x01;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x80;
	fwrite(&charbuf, sizeof(char), 1, psexe);

/*
 * Global Pointer
 */
	charbuf = gp & 0xff;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (gp & 0xff00) >> 8;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (gp & 0xff0000) >> 16;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (gp & 0xff000000) >> 24;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	
/*
 * Seek output file to 0x18, Text section start address
 */
	fseek(psexe, 0x18, SEEK_SET);
 
/*
 * Write text section start address = 0
 */ 
	charbuf = 0x00;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x00;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x01;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x80;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	
/*
 * Seek output file to 0x30, Initial Stack Pointer
 */
	fseek(psexe, 0x30, SEEK_SET);
	
/*
 * Write Initial Stack Pointer = 0x801FFFF0 
 */
	charbuf = 0xF0;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0xFF;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x1F;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = 0x80;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	
	
/*
 * Seek output to 0x4C, ASCII marker
 */
	fseek(psexe, 0x4C, SEEK_SET);
	
	x = 0;
	
/*
 * Write ASCII marker string
 */
    while(psexe_marker[x])
		fwrite(&psexe_marker[x++], sizeof(char), 1, psexe);

/*
 * Run objcopy now
 */
	sprintf(stringbuf, OBJCOPY_PATH" -O binary %s %s.bin", argv[1], argv[1]);
	system(stringbuf);
		
	sprintf(stringbuf, "%s.bin", argv[1]);

/*
 * Open objcopy output
 */
	
	objcopy_out = fopen(stringbuf, "rb");
	if(objcopy_out == NULL)
	{
		printf("Could not open objcopy output at %s. Check your permissions. Aborting.\n",
			stringbuf);
		return -1;
	}
	
/*
 * Seek to 0x800, Program Start
 * and write the output of objcopy into the PS-X EXE
 */
	fseek(psexe, 0x800, SEEK_SET);
	
	while(!feof(objcopy_out))
	{
		x = fgetc(objcopy_out);
		fputc(x, psexe);
	}	
		
	
	fclose(objcopy_out);

/*
 * Get the file size of the PS-X EXE
 */	
	fseek(psexe, 0, SEEK_END);
	sz = ftell(psexe);
	fseek(psexe, 0, SEEK_SET);
	
	if(sz % 2048 != 0)
	{
		fseek(psexe, (((sz / 2048) + 1)*2048) - 1, SEEK_SET);
		fwrite(&charbuf, sizeof(char), 1, psexe);
		sz = ftell(psexe);
	}

/*
 * Write the address of the text section in the header of the PS-X EXE
 */
	
	sz -= 0x800;
	
	fseek(psexe, 0x1C, SEEK_SET);
	
	charbuf = sz & 0xff;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (sz & 0xff00) >> 8;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (sz & 0xff0000) >> 16;
	fwrite(&charbuf, sizeof(char), 1, psexe);
	charbuf = (sz & 0xff000000) >> 24;
	fwrite(&charbuf, sizeof(char), 1, psexe);
		
	fclose(psexe);

/*
 * Remove objcopy output
 */
	sprintf(stringbuf, "%s.bin", argv[1]);
	remove(stringbuf);
	
	return 0;
}

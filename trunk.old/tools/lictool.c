/*
 * lictool
 * 
 * Tool for manipulating PS1 license files
 */
 
#include <stdio.h>
#include <string.h>

unsigned char lic_buffer[37632]; // 16 CD sectors..

//0x2E08

void display_usage();

void display_usage()
{
	printf(""
	"lictool - PS1 license file manipulation tool\n"
	"usage: lictool <input> <output> <options>\n"
	"\n"
	"Options:\n"
	" -tmd=<file>      TMD file for boot logo\n"
	"");
}

int main(int argc, char *argv[])
{
	int x,y,z,sz;
	FILE *f;
	
	if(argc < 3)
	{
		display_usage();
		return 0;
	}
	
	f = fopen(argv[1], "rb");
	
	if(f == NULL)
	{
		printf("Could not open input license file! Aborting.\n");
		return -1;
	}
	
	fread(lic_buffer, sizeof(char), 37632, f);
	fclose(f);
	
	for(x = 3; x < argc; x++)
	{
		if(strncmp(argv[x], "-tmd=", 5) == 0)
		{
			f = fopen(argv[x] + 5, "rb");
			
			if(f == NULL)
				printf("Could not open TMD file %s. Ignoring option.\n", argv[x] + 5);
			else
			{				
				fseek(f, 0, SEEK_END);
				sz = ftell(f);
				fseek(f, 0, SEEK_SET);
				z = 0x2E08;
				
				for(y = 0; y < sz; y++)
				{
					if((z - ((z / 2352)*2352)) == 2072)
						z+=304;
					
					fread(&lic_buffer[z], sizeof(char), 1, f);
					
					z++;
				}
			
				fclose(f);
			}
		}
	}
	
	f = fopen(argv[2], "wb");
	
	if(f == NULL)
	{
		printf("Could not open output file path for writing! Aborting.\n");
		return -1;
	}
	
	fwrite(lic_buffer, sizeof(char), 37632, f);
	fclose(f);
	
	return 0;
}

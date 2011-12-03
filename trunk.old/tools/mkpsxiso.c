/*
 * mkpsxiso
 *
 * Converts an ISO to a .bin/.cue of a Playstation disk
 */

#include <stdio.h>
#include <string.h>

char iso2raw_sec[16];
char iso2raw_sub[8];
char iso2raw_buf[2048];
char iso2raw_edc[4];
char iso2raw_ecc[276];

void Iso2Raw_init()
{
	int x;

	for(x = 0; x < 16; x++)
		iso2raw_sec[x] = 0xFF;
		
	iso2raw_sec[0]  = 0;
    iso2raw_sec[11] = 0;
    iso2raw_sec[12] = 0;
    iso2raw_sec[13] = 2;
    iso2raw_sec[14] = 0;
    iso2raw_sec[15] = 2;
	
	for(x = 0; x < 8; x++)
		iso2raw_sub[x] = 0;
		
	for(x = 0; x < 4; x++)
		iso2raw_edc[x] = 1;
		
	for(x = 0; x < 276; x++)
		iso2raw_ecc[x] = 2;
}

int Iso2Raw_licenseFile(char *licFile, char *binFile) {
	FILE *lic, *bin;
	char buffer[37632];
	int sz;
	int ret;
	  
    lic = fopen(licFile, "rb"); //lic = new RandomAccessFile(licFile, "r");
		 
	if(lic == NULL)
	{
		printf("Error! Could not open license file!\n");
		ret = 0;
		goto Iso2Raw_licenseFile_end;
	}
		 
	bin = fopen(binFile, "rb+"); //bin = new RandomAccessFile(binFile, "rw");
         
	if(bin == NULL)
	{
		printf("Error! Could not open BIN file!\n");
		fclose(lic);
		ret = 0;
		goto Iso2Raw_licenseFile_end;
	}
		 
	fseek(lic, 0, SEEK_END);
	sz = ftell(lic);
	fseek(lic, 0, SEEK_SET);
		 
		 
	if (sz != 37632)
	{
		printf("Error! License file size mismatch. Image not licensed!\n");
		fclose(lic);
		fclose(bin);
		ret = 0;
		goto Iso2Raw_licenseFile_end;
	}
         
	fseek(bin, 0, SEEK_END);
	sz = ftell(bin);
	fseek(bin, 0, SEEK_SET);
		 
	if ((sz % 2352) != 0)
	{
		printf("Error! RAW image file size is not a multiple of 2352. Image not licensed!\n");
		fclose(lic);
		fclose(bin);
		ret = 0;
		goto Iso2Raw_licenseFile_end;
	}
         
	fread(buffer, sizeof(char), 37632, lic);
	fwrite(buffer, sizeof(char), 37632, bin);
   
	fclose(lic);
	fclose(bin);
	ret = 1;
	  
Iso2Raw_licenseFile_end:
	  if(ret == 0)
		printf("Error licensing file! You must NOT burn the RAW image!\n");
	  
      return ret;
}
   
void Iso2Raw_generateCue(char *binFileName)
{
	int x, y;
	char binBaseName[256];
	char cueFileName[256];
	FILE *cue_file;
	
	for(x = (strlen(binFileName) - 1); x >= 0; x--)
	{
		if(binFileName[x] == '/' || binFileName[x] == '\\' || binFileName[x] == ':')
			break;
	}
	
	x++;
	y = 0;
		
	for(; x < strlen(binFileName); x++)
		binBaseName[y++] = binFileName[x];
		
	binBaseName[y] = 0;	
		
	y = 0;
	
	for(x = 0; x < strlen(binFileName); x++)
	{
		if(binFileName[x] == '.')
			break;
		else
			cueFileName[y++] = binFileName[x];
	}
	
	cueFileName[y] = 0;
	
	strcat(cueFileName, ".cue");
		
	cue_file = fopen(cueFileName, "wb");
	
	fprintf(cue_file, "FILE \"%s\" BINARY\n", binBaseName);
	fprintf(cue_file, "TRACK 01 MODE2/2352\n");
    fprintf(cue_file, "    INDEX 01 00:00:00\n");
	
	fclose(cue_file);
}

int Iso2Raw_convert(char *isofile, char *rawfile, char *licfile)
{
	FILE *infile, *outfile;
	int c;
	int thesec = 0;
	int filesize, totalsectors, sector;  
	
	infile = fopen(isofile, "rb");
	  
	if(infile == NULL)
	{
		printf("An error has occured while trying to open file %s\n", isofile);
		return 0;
	}
	  
	fseek(infile, 0, SEEK_END);
	filesize = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	  
	if ((filesize % 2048) != 0)
	{
		printf("Error! ISO file size is not a multiple of 2048. Operation aborted!\n");
		fclose(infile);
		return 0;
	}
	
	outfile = fopen(rawfile, "wb+");
	fseek(outfile, 0, SEEK_SET);
	  
	if(outfile == NULL)
	{
		printf("An error has occured while trying to create file %s\n", rawfile);
		fclose(infile);
		return 0;
	}

	sector = 1;
	totalsectors = filesize / 2048;
	for(;;)
	{
		c = fread(iso2raw_buf, sizeof(char), 2048, infile);
		if(c!=2048)break;
	
		fwrite(iso2raw_sec, sizeof(char), 16, outfile);
		fwrite(iso2raw_sub, sizeof(char), 8, outfile);
		fwrite(iso2raw_buf, sizeof(char), 2048, outfile);
		fwrite(iso2raw_edc, sizeof(char), 4, outfile);
		fwrite(iso2raw_ecc, sizeof(char), 276, outfile);

		thesec++;
            
		if (thesec > 74)
		{
			thesec = 0;
			iso2raw_sec[13]++;
		}
			
		iso2raw_sec[14] = ((thesec/10)<<4)|(thesec - ((thesec/10)*10));
            
		printf("\r%d%% completed...", sector * 100 / totalsectors);
        sector++;
	}
         
	printf("\r100%% completed!  \n");
	
	fclose(infile);
	fclose(outfile);
      
	Iso2Raw_generateCue(rawfile);

	if(!Iso2Raw_licenseFile(licfile, rawfile))
		return 0;

	return 1;
}

int main(int argc, char *argv[])
{      
	puts("mkpsxiso (C Edition) v0.1b - Converts a standard ISO image to .bin/.cue (PSX)");
	puts("This software is based on Bruno Freitas' mkpsxiso in Java - bootsector@ig.com.br");
	puts("That version is in turn based on Conyers' mkpsxiso - http://www.conyers.demon.co.uk");
	puts("Author: Giuseppe Gatta (aka nextvolume) - 01/07/2009 - tails92@gmail.com\n");

	if(argc != 4)
	{
		printf("Usage: mkpsxiso <iso file> <bin file> <PSX license file>\n");
		return 1;
	}

	Iso2Raw_init();
  
	if (!Iso2Raw_convert(argv[1], argv[2], argv[3]))
	{
		puts("ISO file conversion failed.");
		return 1;
	}

	puts("ISO file conversion terminated successfully!!");
      
	return 0;
}

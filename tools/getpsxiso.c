// Converts a bin suitable for burning for the PlayStation
// to an ISO by getting only the 2048 data bytes of each sector
// The reverse of mkpsxiso

// Written by Giuseppe Gatta, 2010

#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *i, *o;
	int x, s;
	char buf[2352];

	if (argc < 3)
	{
		printf("getpsxiso <input> <output>\n");
		return -1;
	}
	
	i = fopen(argv[1], "rb");
	
	if(i == NULL)
	{
		printf("Could not open specified input file.\n");
		return -1;
	}
	
	fseek(i, 0, SEEK_END);
	s = ftell(i) / 2352;
	fseek(i, 0, SEEK_SET);
	
	if(s % 2352 == 0)
	{
		printf("Input file size not a multiplier of 2352.\n");
		printf("Aborting.\n");
		return -1;
	}
	
	o = fopen(argv[2], "wb");
	
	for(x = 0; x < s; x++)
	{
		fread(buf, sizeof(char), 2352, i);
		fwrite(buf + 24, sizeof(char), 2048, o);
		printf("Sector %d/%d written\r", x, s);
	}
	
	printf("\n");
	
	fclose(i);
	fclose(o);
	
	return 0;
}

unsigned short read_le_word(FILE *f)
{
	unsigned char c;
	unsigned short i;
	
	fread(&c, sizeof(char), 1, f);
	i = c;
	fread(&c, sizeof(char), 1, f);
	i|=(c<<8);

	return i;
}

unsigned int read_le_dword(FILE *f)
{
	unsigned char c;
	unsigned int i;
	
	fread(&c, sizeof(char), 1, f);
	i = c;
	fread(&c, sizeof(char), 1, f);
	i|=(c<<8);
	fread(&c, sizeof(char), 1, f);
	i|=(c<<16);
	fread(&c, sizeof(char), 1, f);
	i|=(c<<24);

	return i;
}


void write_le_word(FILE *f, unsigned short leword)
{
	unsigned char c;
	
	c = leword & 0xff;
	fwrite(&c, sizeof(char), 1, f);
	c = leword >> 8;
	fwrite(&c, sizeof(char), 1, f);
}

void write_le_dword(FILE *f, unsigned int ledword)
{
	unsigned char c;
	int x;
	
	for(x = 0; x < 4; x++)
	{
		c = (ledword >> (x<<3)) & 0xff;
		fwrite(&c, sizeof(char), 1, f);
	}
}

/*
 * bmp2tim
 *
 * Converts a bitmap to a TIM image
 * 
 * TEST output in various color depths... and check for issues on big-endian machines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	unsigned char r, g, b;
}PS_RGB;

typedef struct
{
	int w, h;
	int depth;
	void *data;
}PS_BITMAP;

int do_clut = 0;
unsigned int clut_x, clut_y;

unsigned int org_x = 0;
unsigned int org_y = 0;

int tim_depth;

unsigned int tim_flag;

int set_stp_bit = 0;

int transparent_black = 0;
int magic_pink = 0;

PS_RGB *ps_default_palette;

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

PS_BITMAP *ps_create_bitmap(int w, int h, int depth)
{
	PS_BITMAP *bm;
	
	bm = malloc(sizeof(PS_BITMAP));
	
	bm->w = w;
	bm->h = h;
	bm->depth = depth;
	
	switch(depth)
	{
		case 1:
			bm->data = malloc((w*h)/8);
		break;
		case 4:
			bm->data = malloc((w*h)/2);
		break;
		case 8:
			bm->data = malloc(w*h);
		break;
		case 15:
		case 16:
			bm->data = malloc((w*h)*2);
		break;
		case 24:
			bm->data = malloc((w*h)*3);
		break;
		case 32:
			bm->data = malloc((w*h)*4);
		break;
	}
	
	return bm;
}

void ps_destroy_bitmap(PS_BITMAP *bm)
{
	free(bm->data);
	free(bm);
}


PS_BITMAP *ps_load_bitmap(char *filename, PS_RGB *palette)
{
	FILE *bf;
	unsigned int bsize;
	unsigned int bisize;
	unsigned int bwidth, bheight, bbpp, boff, blw;
	unsigned char *bdata;
	PS_BITMAP *bm;
	int x, y, l;
	
	bf = fopen(filename, "rb");
	
	if(bf == NULL)
		return NULL;
		
	if(read_le_word(bf) != 19778) // 'BM'
	{
		fclose(bf);
		return NULL;
	}	
		
	bsize = read_le_dword(bf);
	
// Read bitmap data offset
	fseek(bf, 10, SEEK_SET);
	boff = read_le_dword(bf);
	
// Read information header size, width and height	

	bisize = read_le_dword(bf);
	bwidth = read_le_dword(bf);
	bheight = read_le_dword(bf);
	
// Read BPP
	
	fseek(bf, 28, SEEK_SET);
	
	bbpp = read_le_word(bf);

// Check if there is compression, if there is, abort

	bm = ps_create_bitmap(bwidth, bheight, bbpp);
		
	if(palette != NULL && bm->depth <= 8)
	{
		fseek(bf, 14 + bisize, SEEK_SET);
		
		if(bm->depth == 4) l = 16;
		else if(bm->depth == 8) l = 256;
		else if(bm->depth == 1) l = 2;
		
		for(x=0;x<l;x++)
		{
			palette[x].b = fgetc(bf);
			palette[x].g = fgetc(bf);
			palette[x].r = fgetc(bf);
			fgetc(bf);
		}
	}
	
	blw = bwidth;
	while(blw & 3)
		blw++;
	
	while(bbpp & 7)
		bbpp++;
		
	if(bm->depth == 4)
	{
		blw/=2;
		bwidth/=2;
	}
	else if(bm->depth == 1)
	{
		blw/=8;
		bwidth/=8;
	}
	else
	{
		blw*=(bbpp >> 3);
		bwidth*=(bbpp >> 3);
	}
	
	bdata = (unsigned char*)bm->data;
	
	for(y = 0; y < bm->h; y++)
	{
		fseek(bf, boff + (blw * (bm->h - (1+y))), SEEK_SET);
	
		for(x = 0; x < bwidth; x++)
			fread(&bdata[(y*bwidth)+x], sizeof(char), 1, bf);
	}
	
	fclose(bf);
	
	return bm;
}

unsigned int ps_makecol(int r, int g, int b)
{
	return (r<<16)|(g<<8)|b;
}

unsigned int ps_getr(int c)
{
	return (c>>16)&0xff;
}

unsigned int ps_getg(int c)
{
	return (c>>8)&0xff;
}

unsigned int ps_getb(int c)
{
	return c&0xff;
}

unsigned int ps_getpixel(PS_BITMAP *bm, int x, int y)
{
	unsigned short shortbuf;
	unsigned char r, g, b;
	unsigned char *dataptrb = (unsigned char*)bm->data;
	int off;

	if(bm->depth == 16)
	{
		off = ((y*bm->w)+x)*2;
	
		// Little endian, guys...
	
		shortbuf = dataptrb[off];
		shortbuf|= dataptrb[off+1]<<8;
		
		b = (shortbuf & 0x1f) << 3;
		g = ((shortbuf>>5) & 0x3f) << 2;
		r = ((shortbuf>>11) & 0x1f) << 3;
	
		return ps_makecol(r, g, b);
	}
	else if(bm->depth == 24)
	{
		off = ((y*bm->w)+x)*3;
		r = dataptrb[off+2];
		g = dataptrb[off+1];
		b = dataptrb[off];
		
		return ps_makecol(r,g,b);
	}
	else if(bm->depth == 32)
	{
		off = ((y*bm->w)+x)*4;
		r = dataptrb[off+3];
		g = dataptrb[off+2];
		b = dataptrb[off+1];
	
		return ps_makecol(r,g,b);
	}
	else if(bm->depth == 8)
	{
		r = dataptrb[(y*bm->w)+x];
		
		return ps_makecol(ps_default_palette[r].r, 
			ps_default_palette[r].g, ps_default_palette[r].b);
	}
	else if(bm->depth == 4)
	{
		off = (y*bm->w)+x;
		off/= 2;
		
		if(x & 1)
			r = dataptrb[off] & 0xf;
		else
			r = dataptrb[off] >> 4;
			
		return ps_makecol(ps_default_palette[r].r, 
			ps_default_palette[r].g, ps_default_palette[r].b);
	}
	else if(bm->depth == 1)
	{
		off = (y*bm->w)+x;
		off/= 8;
		
		r = (dataptrb[off] & (1<<(7-(x&7)))) ? 1 : 0;
	
		return ps_makecol(ps_default_palette[r].r,
			ps_default_palette[r].g, ps_default_palette[r].b);
	}
	
	return 0;
}			 

unsigned int ps_getpixel_pal(PS_BITMAP *bm, int x, int y)
{
	unsigned char *dataptrb = (unsigned char*)bm->data;
	int off;

	if(bm->depth == 8)
	{
		return dataptrb[(y*bm->w)+x];
	}
	else if(bm->depth == 4)
	{
		off = (y*bm->w)+x;
		off/= 2;
		
		if(x & 1)
			return dataptrb[off] & 0xf;
		else
			return dataptrb[off] >> 4;
	}
	else if(bm->depth == 1)
	{
		off = (y*bm->w)+x;
		off/= 8;
		
		return (dataptrb[off] & (1<<(7-(x&7)))) ? 1 : 0;
	}
	
	return 0;
}

void parse_options(int argc, char *argv[])
{
	int x;
	
	for(x=4;x<argc;x++)
	{
		if(strncmp("-clut=", argv[x], 6) == 0)
		{
			sscanf(argv[x], "-clut=%d,%d", &clut_x, &clut_y);
			do_clut = 1;
		}else if(strncmp("-org=", argv[x], 5) == 0)
			sscanf(argv[x], "-org=%d,%d", &org_x, &org_y);
		else if(strcmp("-stp", argv[x]) == 0)
			set_stp_bit = 1;
		else if(strcmp("-noblack", argv[x]) == 0)
			transparent_black = 1;
		else if(strcmp("-mpink", argv[x]) == 0)
			magic_pink = 1;
	}
}		

unsigned short rgb24_to_rgbpsx(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned short c;
	
	c = r>>3;
	c|= (g>>3)<<5;
	c|= (b>>3)<<10;
	
	/*if(set_stp_bit) c|=0x8000;*/
// this code is a bit messy, tidy it up.
	
	if(c == 0 && !transparent_black)
		c|=0x8000;
	
	if(c == ((31)|(31<<10)) && magic_pink)
		c=0;
	
	if(set_stp_bit)
	{
		if(transparent_black && c == 0)
			return c;
		
		if(magic_pink && c == ((31)|(31<<10)))
			return c;
		
		c|=0x8000;
	}
	
	return c;
}

int main(int argc, char *argv[])
{
	PS_BITMAP *in_bitmap;
	FILE *out_tim;
	PS_RGB in_palette[256];
	int x, y, c, c2;
	unsigned short shortbuf;
	int cx_out = 0;

	ps_default_palette = in_palette;

	if(argc < 4)
	{
		printf("bmp2tim - converts a bitmap to a TIM image\n");
		printf("usage: bmp2tim <inbmp> <outtim> <depth> [options]\n\n");
		printf("Options:\n");
		printf("  -clut=x,y       - Generate a Color Look Up Table    (default: OFF)\n");
		printf("  -org=x,y        - Set image origin in framebuffer   (default: 0, 0)\n");
		printf("  -stp            - Set semi transparency bit         (default: BLACK ONLY)\n");
		printf("  -noblack        - Make black transparent            (default: OFF)\n");
		printf("  -mpink          - Magic pink, 255,0,255 transparent (default: OFF)\n\n");
		printf("Valid TIM depths are 4 (16-color), 8 (256-color), 16 (RGB555) and 24 (RGB888)\n");
		return -1;
	}
	
	tim_depth = atoi(argv[3]);
	
	parse_options(argc, argv);
	
	if(do_clut && tim_depth >= 16)
	{
		printf("Images with depths higher than 8-bit can't have a color look up table.\n");
		return -1;
	}
	
	if(clut_x & 0xf)
	{
		printf("The X position of the CLUT in the framebuffer must be a multiplier of 16.\n");
		return -1;
	}
	
	switch(tim_depth)
	{
		case 4:
			if(clut_x > (1024-16))
				cx_out = 1;
		break;
		case 8:
			if(clut_x > (1024-256))
				cx_out = 1;
		break;
	}
	
	if(cx_out)
	{
		printf("X position specified for CLUT out of bounds.\n");
		return -1;
	}
	
	if(clut_y >= 512)
	{
		printf("Y position specified for CLUT out of bounds.\n");
		return -1;
	}
	
	if(do_clut)
		printf("Generating a Color Look Up Table (CLUT)\n");
	
	if(tim_depth != 4 && tim_depth != 8 && tim_depth != 16 && tim_depth != 24)
	{
		printf("Invalid color depth specified!\n");
		return -1;
	}
	
	in_bitmap = ps_load_bitmap(argv[1], in_palette);
	
	if(in_bitmap == NULL)
	{
		printf("Unable to load bitmap. Unsupported format or file is unreadable or does not exist.\n");
		return -1;
	}
	
	if(tim_depth == 4 && in_bitmap->depth > 4)
	{
		printf("Error: Only a 4-bit bitmap or a bitmap of lower depth can be used to obtain a 4-bit TIM!\n");
		return -1;
	}
	
	if(tim_depth == 8 && in_bitmap->depth > 8)
	{
		printf("Error: Only a 8-bit or a bitmap of lower depth can be used to obtain a 8-bit TIM!\n");
		return -1;
	}
		
/*	allegro_init();
	set_color_depth(32);
	install_keyboard();
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, in_bitmap->w, in_bitmap->h, 0, 0);
	
	for(y=0;y<in_bitmap->h;y++)
	{
		for(x=0;x<in_bitmap->w;x++)
		{
			c = ps_getpixel_pal(in_bitmap, x, y);
			
			//putpixel(screen, x, y, makecol(ps_getr(c), ps_getg(c), ps_getb(c)));
			putpixel(screen, x, y, makecol(in_palette[c].r, in_palette[c].g,
			in_palette[c].b));
		}
	}
	
	while(!key[KEY_ESC]);*/
	
	if(in_bitmap == NULL)
	{
		printf("Could not open bitmap. Aborting.\n");
		return -1;
	}
	
	if(org_x > (1024-in_bitmap->w))
	{
		printf("X position specified for image data out of bounds.\n");
		return -1;
	}
	
	if(org_y > (512-in_bitmap->h))
	{
		printf("Y position specified for image data out of bounds.\n");
		return -1;
	}	
	
	out_tim = fopen(argv[2], "wb");
	
	if(out_tim == NULL)
	{
		printf("Couldn't open file at path %s for writing. Aborting.\n", argv[2]);
		return -1;
	}
	
	write_le_dword(out_tim, 0x10); /* ID = 0x10, Version = 0x00 */
	
	/* 
	 * Now let's fill the TIM flag double word
	 */
	
	/*
	 * Pixel mode (PMODE)
	 */
	
	switch(tim_depth)
	{
		case 4:
			tim_flag = 0;
		break;
		case 8:
			tim_flag = 1;
		break;
		case 16:
			tim_flag = 2;
		break;
		case 24:
			tim_flag = 3;
		break;
	}
	
	/*
	 * Clut flag (CF)
	 */
	
	if(do_clut)tim_flag|=8;
	
	write_le_dword(out_tim, tim_flag);
	
	/*
	 * If we have to write a CLUT now, we have to write its data block
	 */
	
	if(do_clut)
	{
		/*
		 * Let's write the information for the block - we already know
		 * everything about it.
		 */
		 
		switch(tim_depth)
		{
			case 4:
				write_le_dword(out_tim, 44); // Number of bytes contained by the block
				write_le_dword(out_tim, (clut_y<<16)|clut_x); // X, Y position
				write_le_dword(out_tim, (1<<16)|16); // Width = 16, Height = 1
			break;
			case 8:
				write_le_dword(out_tim, 524);
				write_le_dword(out_tim, (clut_y<<16)|clut_x);
				write_le_dword(out_tim, (1<<16)|256); // Width = 256, Height = 1
			break;
		}
		
		/*
		 * Let's write the CLUT data
		 */
		 
		switch(tim_depth)
		{
			case 4:
				for(x = 0; x < 16; x++)
				{
					shortbuf = rgb24_to_rgbpsx(in_palette[x].r, in_palette[x].g,
						in_palette[x].b);
						
					write_le_word(out_tim, shortbuf);
				}
			break;
			case 8:
				for(x = 0; x < 256; x++)
				{
					shortbuf = rgb24_to_rgbpsx(in_palette[x].r, in_palette[x].g,
						in_palette[x].b);
						
					write_le_word(out_tim, shortbuf);
				}
			break;
		}
	}
	
	/*
	 * Write image data block
	 */
	 
	switch(tim_depth)
	{
		case 4:
			x = 12 + ((in_bitmap->w * in_bitmap->h) / 2);
		break;
		case 8:
			x = 12 + (in_bitmap->w * in_bitmap->h);
		break;
		case 16:
			x = 12 + ((in_bitmap->w * in_bitmap->h) * 2);
		break;
		case 24:
			x = 12 + ((in_bitmap->w * in_bitmap->h) * 3);
		break;
	}
	
	write_le_dword(out_tim, x);
	write_le_dword(out_tim, (org_y<<16)|org_x);
	
	switch(tim_depth)
	{
		case 4:
			write_le_dword(out_tim, (in_bitmap->h<<16)|(in_bitmap->w/4));
		break;
		case 8:
			write_le_dword(out_tim, (in_bitmap->h<<16)|(in_bitmap->w/2));
		break;
		case 16:	
			write_le_dword(out_tim, (in_bitmap->h<<16)|in_bitmap->w);
		break;
		case 24:
			write_le_dword(out_tim, (in_bitmap->h<<16)|(in_bitmap->w+
				(in_bitmap->w/2)));
		break;
	}
	
	switch(tim_depth)
	{
		case 24:
			for(y = 0; y < in_bitmap->h; y++)
			{
				for(x = 0; x < in_bitmap->w; x+=2)
				{
					c = ps_getpixel(in_bitmap, x, y);
					c2 = ps_getpixel(in_bitmap, x+1, y);
					
					write_le_word(out_tim, (ps_getg(c)<<8)|ps_getr(c));
					write_le_word(out_tim, (ps_getr(c2)<<8)|ps_getb(c));
					write_le_word(out_tim, (ps_getb(c2)<<8)|ps_getg(c2));
				}
			}
		break;
		case 16:
			for(y = 0; y < in_bitmap->h; y++)
			{
				for(x = 0; x < in_bitmap->w; x++)
				{
					c = ps_getpixel(in_bitmap, x, y);
					shortbuf = rgb24_to_rgbpsx(ps_getr(c), ps_getg(c), ps_getb(c));
					write_le_word(out_tim, shortbuf);
				}
			}
		break;
		case 4:
			for(y = 0; y < in_bitmap->h; y++)
			{
				for(x = 0; x < in_bitmap->w; x+=4)
				{
					shortbuf = 0;
					for(c = 0; c < 4; c++)
						shortbuf |= (ps_getpixel_pal(in_bitmap, x+c, y)&0xf) << (c<<2);
					
					write_le_word(out_tim, shortbuf);
				}
			}
		break;
		case 8:
			for(y = 0; y < in_bitmap->h; y++)
			{
				for(x = 0; x < in_bitmap->w; x+=2)
				{
					shortbuf = 0;
					for(c = 0; c < 2; c++)
						shortbuf |= (ps_getpixel_pal(in_bitmap, x+c, y)&0xff) << (c<<3);
					
					write_le_word(out_tim, shortbuf);
				}
			}
		break;
	}
	
	fclose(out_tim);
	printf("Bitmap converted to TIM file successfully!\n");	
	return 0;		
}

/*
 * TIM2BMP
 * Converts TIM, emulator save state VRAM data and raw PSX data
 * to Windows bitmap format
 *
 * Written by Giuseppe Gatta (a.k.a. nextvolume), part of PSXSDK
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define PCSX_1_5_SAVESTATE_SUPPORT

z_stream strm;
unsigned short *tim_clut;

typedef struct
{
	unsigned int clut_off;
	unsigned int data_off;
	unsigned short w; // Width in 16-bit unit pixels
	unsigned short real_w; // Real width in pixels
	unsigned short h;
	unsigned char bpp;
	unsigned char compr; // Compression - 0 = normal, 1 = GZIP
	unsigned char has_clut;
	unsigned short *clut;
}tim2bmp_info;

tim2bmp_info	tim_info;

#include "endian.c"

void rgbpsx_to_rgb24(unsigned short psx_c, unsigned char *r, 
	unsigned char *g, unsigned char *b)
{
	*r = (psx_c & 31) << 3;
	*g = ((psx_c >> 5)&31) << 3;
	*b = ((psx_c >> 10) &31) << 3;
}

// Returns number of bytes to round image row with.
int write_bitmap_headers(FILE *f, int w, int h, int bpp)
{
	int x;
	int r;
	int ret;
	
	if(bpp == 16)
		bpp = 24;
	
	fputc('B', f);
	fputc('M', f);
	
	// Calculate and write size of bitmap
	
	if(bpp == 24)
		r = w * 3;
	else if(bpp == 8)
		r = w;
	else if(bpp == 4)
		r = w / 2;
	
	ret = r;
	
	if(r & 3)
	{
		r|=3;
		r++;
	}
	
	ret = r-ret;
	
	x=r*h;
	x+=54;
	
	if(bpp == 8)
		x+= 1024;
	else if(bpp == 4)
		x+= 64;
	
	write_le_dword(f, x);
	
	// Write bfReserved1 and bfReserved2 as zero
	write_le_dword(f, 0);
	
	// Calculate and write data offset in file
	
	x = 54;
	
	if(bpp == 8)
		x+= 1024;
	else if(bpp == 4)
		x+= 64;
	
	write_le_dword(f, x);
	
	write_le_dword(f, 40);
	write_le_dword(f, w); // Width
	write_le_dword(f, h); // Height
	write_le_word(f, 1);
	write_le_word(f, bpp); // Bits Per Pixel
	write_le_dword(f, 0);
	write_le_dword(f, r * h); // Image data size
	write_le_dword(f, 0);
	write_le_dword(f, 0);
	write_le_dword(f, 0);
	write_le_dword(f,0);
	
	return ret;
}

int tim2bmp_read_tim(char *ip, tim2bmp_info *t)
{
	int tim_pmode;
	//int tim_w, tim_h;
	int tim_x, tim_y, tim_cx, tim_cy, tim_cw, tim_ch;
	int bl;
	int x;
	FILE *i = fopen(ip, "rb");
	
	fseek(i,0,SEEK_SET);
	
	if(read_le_dword(i) != 0x10)
	{
		fclose(i);
		return -1;
	}

	tim_pmode = read_le_dword(i);
	t->has_clut = (tim_pmode & 8) ? 1 : 0;
	tim_pmode &= 7;
	
	if(tim_pmode == 0) t->bpp = 4;
	else if(tim_pmode == 1) t->bpp = 8;
	else if(tim_pmode == 2) t->bpp = 16;
	else if(tim_pmode == 3) t->bpp = 24;
	
	if(t->has_clut)
	{
		t->clut_off = 8;
		bl = read_le_dword(i);
		tim_cx = read_le_word(i);
		tim_cy = read_le_word(i);
		tim_cw = read_le_word(i);
		tim_ch = read_le_word(i);
		
		t->clut = malloc((tim_cw * tim_ch) * sizeof(short));
		
		for(x = 0; x < (tim_cw * tim_ch); x++)
			t->clut[x] = read_le_word(i);		
	}
	
	bl = read_le_dword(i);
	
	// Read framebuffer X,Y coordinates
	
	tim_x = read_le_word(i); 
	tim_y = read_le_word(i);
	
	// Read width and height
	t->w = read_le_word(i); // Fix this for 4bpp and 8bpp images !
	t->h = read_le_word(i);
	
	switch(tim_pmode)
	{
		case 0: t->real_w = t->w * 4; break;
		case 1: t->real_w = t->w * 2; break;
		case 2:
			t->real_w = t->w;
		break;
	}
	
	t->data_off = ftell(i);
	t->compr = 0;
	
	fclose(i);
	
	return 1;
}

int tim2bmp_read_pcsx15(char *ip, tim2bmp_info *t)
{
	t->w = 1024;
	t->real_w = 1024;
	t->h = 512;
	t->bpp = 16;
	t->has_clut = 0;
	t->clut_off = 0;
	t->data_off =  0x2996C0;
	t->compr = 1;

	return 1;
}

void tim2bmp_convert_image_data(char *ip, char *fp, tim2bmp_info *t)
{
	int row_round;
	int y,x;
	//int z;
	int tim_row_off;
	unsigned short c;
	unsigned char r, g, b;
	gzFile gzf;
	//unsigned char test[17];
	FILE *i = fopen(ip, "rb");
	FILE *f = fopen(fp, "wb");
	
	if(t->compr == 1)
		gzf = gzopen(ip, "rb");
	
	write_bitmap_headers(f, t->real_w, t->h, t->bpp);
	
	if(t->has_clut)
	{
		if(t->bpp == 4) // 4bpp
		{
			for(x = 0;  x < 16; x++)
			{
				rgbpsx_to_rgb24(t->clut[x], &r, &g, &b);
				fputc(b, f);
				fputc(g, f);
				fputc(r, f);
				fputc(0, f);
			}
		}
		else if(t->bpp == 8) // 8 bpp
		{
			for(x = 0;  x < 256; x++)
			{
				rgbpsx_to_rgb24(t->clut[x], &r, &g, &b);
				fputc(b, f);
				fputc(g, f);
				fputc(r, f);
				fputc(0, f);
			}
		}
	}
	else
	{
		if(t->bpp == 4) fseek(f, 64, SEEK_CUR);
		else if(t->bpp == 8) fseek(f, 1024, SEEK_CUR);
	}

	if(t->bpp == 16)
		y = (t->real_w * 24) / 8;
	else
		y = (t->real_w * t->bpp) / 8;
	
//	printf("y = %d\n", y);
	row_round = y;
//	printf("row_round = %d\n", y);
	
	if(row_round & 3)
	{
		row_round |= 3;
		row_round++;
	}
//		printf("row_round = %d\n", y);

	row_round -= y;
//	printf("row_round = %d\n", row_round);
	
	for(y = 0; y < t->h; y++)
	{
		tim_row_off = (t->w * 2) * ((t->h - 1)-y);
		
		if(t->compr == 1)
			gzseek(gzf, t->data_off + tim_row_off, SEEK_SET);
		else
			fseek(i, t->data_off + tim_row_off, SEEK_SET);
		
		//		printf("ERRNO SHY = %s\n", gzerror(gzf, &x));

		
		for(x = 0; x < t->w; x++)
		{
			if(t->compr == 1)
			{
				gzread(gzf, &b, 1);
				c = b;
				gzread(gzf, &b, 1);
				c|=b<<8;
			}
			else
				c = read_le_word(i);
			
			switch(t->bpp)
			{
				case 4:
					fputc(((c >> 4) & 0xf) | ((c & 0xf) << 4), f);
					fputc(((c >> 12) & 0xf) | (((c>>8)&0xf)<<4), f);
				break;
				case 8:
					write_le_word(f, c);
				break;
				case 16:
					rgbpsx_to_rgb24(c, &r, &g, &b);
			
					fputc(b, f);
					fputc(g, f);
					fputc(r, f);
				break;
			}
		}
		
		for(x = 0; x < row_round; x++)
			fputc(0, f);
	}
	
	fclose(i);
	fclose(f);
	if(t->compr == 1) gzclose(gzf);
}

int main(int argc, char *argv[])
{
	//int x, y;
	FILE *i;
	//int bl;
	/*int tim_w, tim_h, tim_x, tim_y, tim_cx, tim_cy, tim_cw, tim_ch;
	int row_round;
	unsigned short c;*/
	//unsigned char	r, g, b;
	/*int tim_pdata_fpos;
	int tim_row_off;
	int tim_pmode;
	int tim_has_clut;
	int actual_w;
	int bmp_bpp;*/
	int r;
	tim2bmp_info *t = &tim_info;
	
	if(argc < 2)
	{
		printf("tim2bmp - converts a TIM image to a bitmap\n");
		printf("usage: tim2bmp <intim> <outbmp> [options]\n");
		printf("\n");
		printf("Options:\n");
		printf("  -o=<offset>\n");
		printf("\n");
		return -1;
	}
	
	/*strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	
	r = deflateInit(&strm, 1);*/
	//printf("r = %d, Z_OK = %d\n", r, Z_OK);
	
	i = fopen(argv[1], "rb");
	
	if(i == NULL)
	{
		printf("Couldn't open specified TIM file for reading.\n");
		return -1;
	}
	
	fclose(i);
	
	r = tim2bmp_read_tim(argv[1], &tim_info);
	
	if(r != 1)
		r = tim2bmp_read_pcsx15(argv[1], &tim_info);
		
	if(argc > 2)
		tim2bmp_convert_image_data(argv[1], argv[2], t);

	return 0;
}

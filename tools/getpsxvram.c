#include <allegro.h>
#include <stdio.h>

int show_info = 0;
int kp[16];
int src_x = 0, src_y = 0;

unsigned char vram_buf[0x100000];
BITMAP *vram_bmp;

void render_vram_bmp15()
{
	unsigned short w;
	unsigned char r, g, b;
	int x, y;

	clear(vram_bmp);

	for(y=0;y<512;y++)
	{
		for(x=0;x<1024;x++)
		{
			w = vram_buf[(y<<11)+(x<<1)];
			w|= vram_buf[(y<<11)+(x<<1)+1]<<8;
			
			r = w&0x1f;
			r <<= 3;
			g = (w>>5)&0x1f;
			g <<= 3;
			b = (w>>10)&0x1f;
			b <<= 3;
			
			putpixel(vram_bmp, x, y, makecol(r,g,b));
		}
	}
}

void render_vram_bmp8()
{
	unsigned char c;
	int x, y;

	clear(vram_bmp);

	for(y=0;y<512;y++)
	{
		for(x=0;x<2048;x++)
		{
			c = vram_buf[(y<<11)+x];
			putpixel(vram_bmp, x, y, makecol(c,c,c));
		}
	}
}	

void render_vram_bmp4()
{
	unsigned char c;
	int x, y;

	clear(vram_bmp);

	for(y=0;y<512;y++)
	{
		for(x=0;x<4096;x+=2)
		{
			c = vram_buf[(y<<11)+(x>>1)];
			
			putpixel(vram_bmp, x, y, makecol((c&0xf)<<4,(c&0xf)<<4,(c&0xf)<<4));
			putpixel(vram_bmp, x+1, y, makecol(c&0xf0,c&0xf0,c&0xf0));
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *i,*o;
	
	allegro_init();
	install_keyboard();
	
	if(argc < 4)
	{
		printf("getpsxvram\n");
		printf("Gets PlayStation Video RAM from save states\n");
		printf("usage: getpsxvram [state] [outfile] [outfmt]\n");
		printf("\n");
		printf("Where outfmt can be:\n");
		printf("raw    - Dumps Video RAM as it is\n");
		printf("bmp15  - Renders Video RAM in 15-bpp mode and\n");
		printf("         saves bitmap to output file\n");
		printf("bmp8   - Renders Video RAM in 8-bpp mode and\n");
		printf("         saves bitmap to output file\n");
		printf("bmp4   - Renders Video RAM in 4-bpp mode and\n");
		printf("         saves bitmap to output file\n");
		printf("\n");
		printf("For more information, check README.txt\n");
		return -1;
	}
	
	i = fopen(argv[1], "rb");
	
	if(i == NULL)
	{
		printf("Could not open input file.\nAborting.\n");
		return -1;
	}
	
// For this is hardcoded to PCSX 1.5 savestates
// then add support for ePSXe 1.6.0/1.7.0 and PCSX forks	
	
	//fseek(i, 0x2992B8+0x408, SEEK_SET);
	fseek(i, 0x2996C0, SEEK_SET);
	
	fread(vram_buf, sizeof(char), 0x100000, i);
	
	fclose(i);
	
	if(strcmp(argv[3], "raw") == 0)
	{
		o = fopen(argv[2], "wb");
		fwrite(vram_buf, sizeof(vram_buf), 1, o);
		fclose(o);
	}
	else if(strcmp(argv[3], "bmp15") == 0)
	{
		set_color_depth(24);
		vram_bmp = create_bitmap(1024, 512);
		render_vram_bmp15();
		save_bitmap(argv[2], vram_bmp, default_palette); 
	}
	else if(strcmp(argv[3], "bmp8") == 0)
	{
		set_color_depth(24);
		vram_bmp = create_bitmap(2048, 512);
		render_vram_bmp8();
		save_bitmap(argv[2], vram_bmp, default_palette);
	}
	else if(strcmp(argv[3], "bmp4") == 0)
	{
		set_color_depth(24);
		vram_bmp = create_bitmap(4096, 512);
		render_vram_bmp4();
		save_bitmap(argv[2], vram_bmp, default_palette);
	}else
	{
		printf("Output format %s unknown.\nAborting.\n", argv[3]);
		return -1;
	}
	
	return 0;
}
END_OF_MAIN()


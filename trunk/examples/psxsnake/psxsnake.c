/*
 * Snake clone for the PSX
 */

#include <psx.h>
#include <stdio.h>

int vmode;
int speed_counter = 0;
int screen_old = 0;

int snake_array[29][40];
int scc = 0;

int snake_size = 3;
int game_over = 0;

int rectangle_mode = 0;
int l1_pressed = 0;

int level_number = 1;
int score = 0;
int seed_counter = 0;

unsigned int game_draw_list[0x4000]; /* 128 kilobytes */

enum
{
	SNAKE_DIR_LEFT, SNAKE_DIR_RIGHT, SNAKE_DIR_UP, SNAKE_DIR_DOWN
};

int snake_dir = SNAKE_DIR_RIGHT;

unsigned char file_buffer[0x30000]; /* 192 kilobytes */

void game_init();
void game_vblank_handler();
void game_run();
void game_run_gameover();
int pal_or_ntsc_selection();
int main();
int check_snake_collision(int x, int y);
void load_file_into_buffer(char *fname);
void game_print(char *string, int x, int y);
void game_center_print(char *string, int x, int y);
void setup_snake_field();
void game_setup();

GsDrawEnv game_drawenv;
GsDispEnv game_dispenv;

GsImage game_image;
GsSprite game_sprite;
GsRectangle game_rect;

unsigned short game_clut[16];

char string_buf[256];

int cross_pressed = 0;
int circle_pressed = 0;

void game_init()
{
// Initialize the PSXSDK library
	PSX_Init();
	
// Initialize graphics
	GsInit();
	
// Clear video memory	
	GsClearMem();

// Set up drawing environment
	game_drawenv.dither = 0;
	game_drawenv.draw_on_display = 0;
	game_drawenv.x = 0;
	game_drawenv.y = 0;
	game_drawenv.w = 320;
	game_drawenv.h = 240;
	game_drawenv.ignore_mask = 0;
	game_drawenv.set_mask = 0;
	
	GsSetDrawEnv(&game_drawenv);
	
// Set up display environment
	game_dispenv.x = 0;
	game_dispenv.y = 256;
	
	GsSetDispEnv(&game_dispenv);
	
// Set drawing list
	GsSetList(game_draw_list);
}

void game_vblank_handler()
{
	speed_counter++;
	seed_counter++;
	screen_old = 1;
}

int pal_or_ntsc_selection()
{
	unsigned short padbuf;
	int x;
	char stringbuf[64];

	game_drawenv.draw_on_display = 1;
	x = game_drawenv.y;
	game_drawenv.y = game_dispenv.y;
	GsSetDrawEnv(&game_drawenv);

	game_rect.x = 0;
	game_rect.y = 0;
	game_rect.w = 320;
	game_rect.h = 240;
	game_rect.r = 0;
	game_rect.g = 0;
	game_rect.b = 0;
	game_rect.attribute = 0;
			
	GsSortRectangle(&game_rect);

	game_print("PAL/NTSC SELECTION", 128, 64);
	game_print("X - PAL", 128, 80);
	game_print("O - NTSC", 128, 88);

	GsDrawList();

	while(GsIsDrawing());
	
	game_drawenv.draw_on_display = 0;
	game_drawenv.y = x;
	GsSetDrawEnv(&game_drawenv);

	while(1)
	{
		PSX_ReadPad(&padbuf, NULL);
		
		if((padbuf & PAD_CROSS) && !cross_pressed)
			return VMODE_PAL; // PAL
			
		if((padbuf & PAD_CIRCLE) && !circle_pressed)
		{
			//printf("circle_pressed = %d\n", circle_pressed);
			return VMODE_NTSC; // NTSC
		}
			
			
		if(!(padbuf & PAD_CROSS))
			cross_pressed = 0;
			
		if(!(padbuf & PAD_CIRCLE))
			circle_pressed = 0;
	}
}

void new_apple()
{
	int a, b;

	do
	{
		while((a = rand()%40) < 1);
		srand(seed_counter);
		while((b = rand()%29) < 2);
		srand(seed_counter);
	}while(snake_array[b][a]);
		
	snake_array[b][a] = 0x80;
}

int main()
{
	FILE *f;
	int c;

// Initialize
	game_init();
	
	printf("PSXsnake\n");
	printf("(c) 2009 Giuseppe Gatta\n");
	printf("Made with PSXSDK\n\n");
	
	load_file_into_buffer("cdrom:FONT.TIM;1");
	GsImageFromTim(&game_image, file_buffer);
	
	GsUploadImage(&game_image);
	
	game_clut[0] = 0x0;
	game_clut[1] = 0x7fff;
	
	LoadImage(game_clut, 640, 24, 16, 1);
	
	GsSetVideoMode(320, 240, EXAMPLES_VMODE);
	
	vmode = pal_or_ntsc_selection();
	
	//printf("vmode = %d\n", vmode);
	
	GsSetVideoMode(320, 240, vmode);
	
	game_setup();
	
	load_file_into_buffer("cdrom:BACKGRND.TIM;1");
	GsImageFromTim(&game_image, file_buffer);
	
	GsUploadImage(&game_image);
	
	SetVBlankHandler(game_vblank_handler);
	
	while(1)
		game_run();
		
	return 0;
}

int check_snake_collision(int x, int y)
{
	if(snake_array[y][x] == 0x80)
		return 2; // Snake ate an apple
	else if(snake_array[y][x] == 0)
		return 1; // Collided with nothing
		
		
	return 0; // Die, because snake collided with itself
}

void game_run()
{
	int x, y, a, b, c;

	unsigned short padbuf;

	while(speed_counter > 0)
	{
		scc++;
		
		PSX_ReadPad(&padbuf, NULL);
		
		if(scc == 5 && !game_over)
		{
			if(snake_dir <= SNAKE_DIR_RIGHT)
			{
				if(padbuf & PAD_UP)
					snake_dir = SNAKE_DIR_UP;
							
				if(padbuf & PAD_DOWN)
					snake_dir = SNAKE_DIR_DOWN;
			}
			else
			{	
				if(padbuf & PAD_LEFT)	
					snake_dir = SNAKE_DIR_LEFT;
							
				if(padbuf & PAD_RIGHT)
					snake_dir = SNAKE_DIR_RIGHT;
			}
			
			for(y = 0; y < 29; y++)
				for(x = 0; x < 40; x++)
				{
					if(snake_array[y][x] == snake_size)
					{
						switch(snake_dir)
						{
							case SNAKE_DIR_LEFT:
								b = y;
								a = x-1;
							break;
							case SNAKE_DIR_RIGHT:
								b = y;
								a = x+1;
							break;								
							case SNAKE_DIR_UP:
								b = y-1;
								a = x;
							break;
							case SNAKE_DIR_DOWN:
								b = y+1;
								a = x;
							break;
						}
						
						c = check_snake_collision(a,b);
						
						if(c)
						{
							snake_array[b][a] = snake_size+1;
							
							if(c==2)
							{
								snake_size++;
								score+=100;
								//printf("%d\n", score);
								new_apple();
							}
						}
						else
						{
							game_over = 1;
							scc = 0;
						}
						
						if(snake_array[y][x] == 1 && c!=2)
							snake_array[y][x] = 0;
						
						goto out_of_collision_checking;
					}
				}
out_of_collision_checking:				
			for(y = 0; y < 29; y++)
				for(x = 0; x < 40; x++)
					if(snake_array[y][x]&&snake_array[y][x]<0x80&&c!=2)
						snake_array[y][x]--;
			
			scc = 0;
		}
		else if(game_over)
		{
			scc++;
			
			if(scc >= 510)
			{
				if((padbuf & PAD_CROSS) && !cross_pressed)
				{
					game_setup();
					cross_pressed = 1;
				}
				else if((padbuf & PAD_CIRCLE) && !circle_pressed)
				{
					circle_pressed = 1;
					GsSetVideoMode(320, 240, pal_or_ntsc_selection());
					game_setup();
				}
			
				if(!(padbuf & PAD_CROSS))
					cross_pressed = 0;
				
				if(!(padbuf & PAD_CIRCLE))
					circle_pressed = 0;
			}
		}
			
		speed_counter--;
	}
	

	if(screen_old)
	{
		game_rect.x = 0;
		game_rect.y = 0;
		game_rect.w = 320;
		game_rect.h = 240;
		game_rect.r = 0;
		game_rect.g = 0;
		game_rect.b = 0;
		game_rect.attribute = 0;
			
		GsSortRectangle(&game_rect);
		
		game_sprite.x = 0;
		game_sprite.y = 0;
		game_sprite.w = 256;
		game_sprite.h = 240;
		game_sprite.u = 0;
		game_sprite.v = 0;
		game_sprite.r = NORMAL_LUMINOSITY;
		game_sprite.g = NORMAL_LUMINOSITY;
		game_sprite.b = NORMAL_LUMINOSITY;
		game_sprite.tpage = 5;
		game_sprite.attribute = COLORMODE(COLORMODE_16BPP);
		
		GsSortSimpleSprite(&game_sprite);
		
		game_sprite.x += 256;
		game_sprite.w = 64;
		game_sprite.tpage = 9;
			
		GsSortSimpleSprite(&game_sprite);	
			
		game_rect.w = 8;
		game_rect.h = 8;
		game_rect.attribute = ENABLE_TRANS | TRANS_MODE(0);
			
		for(y = 0; y < 29; y++)
		{
			for(x = 0; x < 40; x++)
			{
				game_rect.x = x * 8;
				game_rect.y = y * 8;
					
				if(snake_array[y][x] >= 1 && snake_array[y][x] <= 0x7F)
				{
					game_rect.r = 0;
					game_rect.g = 255;
					game_rect.b = 0;
					GsSortRectangle(&game_rect);
				}
				else if(snake_array[y][x] == 0x80)
				{
					game_rect.r = 255;
					game_rect.g = 0;
					game_rect.b = 255;
					GsSortRectangle(&game_rect);
				}
				else if(snake_array[y][x] == 0x81)
				{
					game_rect.r = 0;
					game_rect.g = 0;
					game_rect.b = 128;
					GsSortRectangle(&game_rect);
				}
			}
		}
		
		sprintf(string_buf, "SCORE: %d", score);
		game_print(string_buf, 0, 232);

		if(game_over)
		{

			game_rect.w = 320;
			game_rect.h = 240;
			
			game_rect.x = 0;
			game_rect.y = 0;
			
			if(scc<=255)x=scc;else x=255;
			
			game_rect.r = x;
			game_rect.g = x;
			game_rect.b = x;
			game_rect.attribute = ENABLE_TRANS|TRANS_MODE(2);
					
			GsSortRectangle(&game_rect);
			
			if(scc>=300)
			{
				game_center_print("GAME OVER!", 160, 120);
			}
			
			if(scc>=420)
				game_center_print("WHAT DO YOU WANT TO DO NOW?", 160, 136);
				
			if(scc>=450)
				game_center_print("PRESS X TO RESTART THE GAME.",160, 152);
			
			if(scc>=480)
				game_center_print("PRESS O FOR PAL/NTSC SELECTION SCREEN.",160,168);	
				
			if(scc>=510)
				game_center_print("MADE WITH PSXSDK BY GIUSEPPE GATTA, 2010", 160, 200);
		}
			
		GsDrawList();
		
// While the graphic synthesizer (video card) is drawing
// just sleep.				
		while(GsIsDrawing());

// Swap drawing and display Y position, and swap list array
// to use. In this way we achieve a double buffer.
		
		if(game_dispenv.y == 0)
		{
			game_dispenv.y = 256;
			game_drawenv.y = 0;
		}
		else
		{
			game_dispenv.y = 0;
			game_drawenv.y = 256;
		}
			
		GsSetDispEnv(&game_dispenv);
		GsSetDrawEnv(&game_drawenv);
			
		screen_old = 0;
	}
}

void game_run_gameover()
{
	printf("GAME OVER!\n");
	while(1);
}

void load_file_into_buffer(char *fname)
{
	FILE *f;
	int sz;

	/*int fd;
	
	fd = open(fname, O_RDONLY);
	
	printf("%s (%d)\n", fname, get_file_size(fname));
	
	read(fd, file_buffer, get_file_size(fname));
	
	close(fd);*/
	f = fopen(fname, "rb");
	
	//f->pos = 0;
	fseek(f, 0, SEEK_END);	

	sz = ftell(f);

	fseek(f, 0, SEEK_SET);

	printf("%s (%d)\n", fname, sz);
	
	fread(file_buffer, sizeof(char), sz, f);
	
	fclose(f);
}

void game_print(char *string, int x, int y)
{
	GsSprite print_char;
	char q;
	
	print_char.x = x;
	print_char.y = y;
	print_char.w = 8;
	print_char.h = 8;
	print_char.r = NORMAL_LUMINOSITY;
	print_char.g = NORMAL_LUMINOSITY;
	print_char.b = NORMAL_LUMINOSITY;
	print_char.cx = 640;
	print_char.cy = 24;
	print_char.tpage = 10;
	print_char.attribute = COLORMODE(COLORMODE_4BPP);

	while(*string)
	{
		if(*string >= 0x20 && *string <= 0x7F)
		{
			q = *string;
			q -= 0x20;
			
			print_char.u = (q&0x1f)<<3;
			print_char.v = (q/0x20)<<3;
			GsSortSimpleSprite(&print_char);
		}
		print_char.x+=8;
		string++;
	}
}

void game_center_print(char *string, int x, int y)
{
	int l = strlen(string);
	l<<=2;

	game_print(string, x - l, y);
}





void setup_snake_field()
{
	int x, y;

	for(y=0;y<29;y++)
		for(x=0;x<40;x++)
			snake_array[y][x] = 0;
	
	for(y=0;y<29;y++)
	{
		snake_array[y][0] = 0x81;
		snake_array[y][39] = 0x81;
	}
	
	for(x=0;x<40;x++)
	{
		snake_array[0][x] = 0x81;
		snake_array[28][x] = 0x81;
	}
	
	new_apple();
	
	for(x=1;x<=snake_size;x++)
		snake_array[1][x]=x;
		
	snake_dir = SNAKE_DIR_RIGHT;
}

void game_setup()
{
	game_over = 0;
	scc = 0;
	snake_size = 3;
	speed_counter = 0;
	score = 0;
	setup_snake_field();
}


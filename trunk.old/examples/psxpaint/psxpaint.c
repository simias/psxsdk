/*
 * PSXPaint
 */

/*
 * Video memory layout:
 *
 * [0-639],[0-479] - Screen memory
 * [640-895],[480-511] - Color Look Up Tables
 * [640-643],[0-23]  - Cursor Image (16x24) (4bpp - real width: 16)
 * [992-1023],[0-31] - Cursor Dirty Rectangle (32x32)
 */

#include <stdio.h>
#include <psx.h>
#include <psxgpu.h>
#include "cursor.h"

int cursor_x = 0, cursor_y = 0, cursor_speed = 1;
int old_cursor_x, old_cursor_y;
volatile unsigned int speed_counter = 0;
int brush_type = 0;
 
unsigned int primitive_list[4000]; 

int paint_colors[20][3] = {
	{ 0, 0, 0 }, // #0 Black
	{ 255, 255, 255 }, // #1 White
	{ 255, 0, 0 }, // #2 Red
	{ 0, 255, 0 }, // #3 Green
	{ 0, 0, 255 }, // #4 Blue
	{ 255, 255, 0}, // #5 Yellow
	{ 255, 0, 255}, // #6 Magenta (Fuchsia)
	{ 0, 255, 255}, // #7 Aqua Green
	{ 255, 128, 0}, // #8 Orange
	{ 128, 255,0},  // #9 Lime Green
	
	{ 192, 192,255},// #10 Sky Blue
	{ 128,128,128}, // #11 Grey
	{ 128, 0,0  },  // #12 Dark Red
	{ 0,  128, 0},  // #13 Dark Green
	{ 0, 0, 128},   // #14 Dark Blue
	{ 128, 128, 0}, // #15 Dark Yellow
	{ 128, 0, 128}, // #16 Purple
	{ 0, 128, 128}, // #17 Dark Aqua
	{ 128, 64, 0},  // #18 Brown
	{ 64, 128, 0},  // #19 Dark Lime Green
	
	/*{ 128, 0, 255}, // #10 Violet
	{ 128,128,128}, // #11 Grey
	{ 128, 0,0  },  // #12 Dark Red
	{ 0,  128, 0},  // #13 Dark Green
	{ 0, 0, 128},   // #14 Dark Blue
	{ 192, 192,255},// #15 Sky Blue
	{ 128, 128, 0}, // #16 Dark Yellow
	{ 150, 75, 0},  // #17 Brown
	{ 64, 64, 64},  // #18 Dark Grey
	{ 128, 255,0},  // #19 Lime Green
	*/
};

int current_color[3];	

void sort_color_boxes()
{
	GsRectangle colorbox;
	int x, y, c=0;
	
	for(y = 384; y < (384+32); y+=16)
	{
		for(x = 0; x < 640; x+=64)
		{
			colorbox.x = x;
			colorbox.y = y;
			colorbox.w = 64;
			colorbox.h = 16;
			colorbox.r = paint_colors[c][0];
			colorbox.g = paint_colors[c][1];
			colorbox.b = paint_colors[c][2];
			colorbox.attribute = 0;
			
			GsSortRectangle(&colorbox);
			
			c++;
		}
	}
}

void load_ui_graphics()
{
	GsImage my_image;

	GsImageFromTim(&my_image, cursor_tim);
 	GsUploadImage(&my_image);
}
 
void my_vblank_handler(); 
 
void my_vblank_handler()
{
	speed_counter++;
} 
 
int main()
{
	GsDispEnv my_dispenv;
	GsDrawEnv my_drawenv;
	GsLine my_line;
	GsSprite my_sprite;
	GsRectangle colorbox;
	GsDot my_dot;
	unsigned short pad1;
	unsigned int WasL2=0, WasR2=0, WasL1=0, WasR1=0, WasSelect = 0;
	int x, y;

	PSX_Init();
	GsInit();
	SetVBlankHandler(my_vblank_handler);

	// This has to be interlaced	
	GsSetVideoModeEx(640, 480, EXAMPLES_VMODE, 0, 1, 0);
	
	my_dispenv.x = 0;
	my_dispenv.y = 0;
	
	GsSetDispEnv(&my_dispenv);
	
	my_drawenv.dither = 0;
	my_drawenv.draw_on_display = 1;
	my_drawenv.x = 0;
	my_drawenv.y = 0;
	my_drawenv.w = 640;
	my_drawenv.h = 512;
	my_drawenv.ignore_mask = 0;
	my_drawenv.set_mask = 0;
	
	GsSetDrawEnv(&my_drawenv);
	
	GsSetList(primitive_list);
	
	load_ui_graphics();
	while(GsIsDrawing());
	
	colorbox.x = 0;
	colorbox.y = 0;
	colorbox.w = 640;
	colorbox.h = 511;
	colorbox.r = 255;
	colorbox.g = 255;
	colorbox.b = 255;
	colorbox.attribute = 0;
	
	GsSortRectangle(&colorbox);
	
	sort_color_boxes();
	
	
	my_sprite.x = 0;
	my_sprite.y = 0;
	my_sprite.tpage = 10;
	my_sprite.u = 0;
	my_sprite.v = 0;
	my_sprite.attribute = 0;
	my_sprite.cx = 640;
	my_sprite.cy = 480;
	my_sprite.r = my_sprite.g = my_sprite.b = NORMAL_LUMINOSITY;
	my_sprite.scalex = my_sprite.scaley = 0;
	my_sprite.w = 16;
	my_sprite.h = 24;
		
	GsDrawList();	
	while(GsIsDrawing());
	
	// Backup 32x32 area
	MoveImage(cursor_x, cursor_y, 992, 0, 32, 32);
	
	my_dot.attribute = 0;
	
	while(1)
	{	
		while(speed_counter)
		{
			old_cursor_x = cursor_x;
			old_cursor_y = cursor_y;
	
			// Restore 32x32 area
			MoveImage(992, 0, old_cursor_x, old_cursor_y, 32, 32);
	
			PSX_ReadPad(&pad1, NULL);
		
			if(pad1 & PAD_LEFT)
				cursor_x-=cursor_speed;
			
			if(pad1 & PAD_RIGHT)
				cursor_x+=cursor_speed;
			
			if(pad1 & PAD_UP)
				cursor_y-=cursor_speed;
		
			if(pad1 & PAD_DOWN)
				cursor_y+=cursor_speed;
			
			if(cursor_x <= 0)
				cursor_x = 0;
			
			if(pad1 & PAD_CROSS)
			{
				if(cursor_y >= 384)
				{
					y = (cursor_y - 384) >> 4;
					x = cursor_x >> 6;
					
					current_color[0] = paint_colors[(y * 10)+x][0];
					current_color[1] = paint_colors[(y * 10)+x][1];
					current_color[2] = paint_colors[(y * 10)+x][2];
				}
				else
				{
				
				switch(brush_type)
				{
					case 0:
						if(cursor_y >= 384)
						{
							break;
						}
					
						my_dot.r = current_color[0];
						my_dot.g = current_color[1];
						my_dot.b = current_color[2];
						my_dot.x = cursor_x;
						my_dot.y = cursor_y;
				
						GsSortDot(&my_dot);
					break;
					case 1:
						if(cursor_y >= 380)
						{
							break;
						}
					
						my_dot.r = current_color[0];
						my_dot.g = current_color[1];
						my_dot.b = current_color[2];
						my_dot.x = cursor_x + 1;
						my_dot.y = cursor_y;
				
						GsSortDot(&my_dot);
						
						my_dot.x++;
						
						GsSortDot(&my_dot);
						
						my_dot.y++;
						my_dot.x-=2;
						
						GsSortDot(&my_dot);
						
						my_dot.x++;
						
						GsSortDot(&my_dot);
						
						my_dot.x++;
						
						GsSortDot(&my_dot);
						
						my_dot.x++;
						
						GsSortDot(&my_dot);
						
						my_dot.y++;
						my_dot.x-=2;
						
						GsSortDot(&my_dot);
						
						my_dot.x++;
						
						GsSortDot(&my_dot);
					break;
				}
				
				}
				
				GsDrawList();
				while(GsIsDrawing());
			}
				
			if((pad1 & PAD_R2) && !WasR2)
			{
				cursor_speed++;
				WasR2 = 1;
			}
				
			if((pad1 & PAD_L2) && !WasL2)
			{
				cursor_speed--;
				WasL2 = 1;
			}
			
			if(!(pad1 & PAD_R2))
				WasR2 = 0;
				
			if(!(pad1 & PAD_L2))
				WasL2 = 0;
			
			if((pad1 & PAD_R1) && !WasR1)
			{
				brush_type++;
				WasR1 = 1;
			}
			
			if((pad1 & PAD_L1) && !WasL1)
			{
				brush_type--;
				WasL1 = 1;
			}
			
			if((pad1 & PAD_SELECT) && !WasSelect)
			{
				my_sprite.attribute ^= (ENABLE_TRANS | TRANS_MODE(0));
				my_dot.attribute ^= (ENABLE_TRANS | TRANS_MODE(0));
				WasSelect = 1;
			}
			
			if(!(pad1 & PAD_SELECT))
				WasSelect = 0;
			
			if(!(pad1 & PAD_R1))
				WasR1 = 0;
				
			if(!(pad1 & PAD_L1))
				WasL1 = 0;
				
			if(cursor_speed <= 0)
				cursor_speed = 1;
				
			if(cursor_speed >= 8)
				cursor_speed = 7;
				
			if(brush_type <= 0)
				brush_type = 0;
				
			if(brush_type > 1)
				brush_type = 1;	
	
			// Backup 32x32 area
				MoveImage(cursor_x, cursor_y, 992, 0, 32, 32);
	
		//	if(cursor_x != old_cursor_x || cursor_y != old_cursor_y)
		//	{
			//	printf("cx = %d, cy = %d, cursor_speed = %d, brush_type = %d\n",
			//	cursor_x, cursor_y, cursor_speed, brush_type);
		

			

		
				my_sprite.x = cursor_x;
				my_sprite.y = cursor_y;
				GsSortSimpleSprite(&my_sprite);
		
				GsDrawList();
				while(GsIsDrawing());
		//	}
			
			speed_counter--;
		}
	}	
}

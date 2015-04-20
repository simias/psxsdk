// PSXSDK Graphics Processing Unit (GPU) / Graphics Synthesizer (GS)
// Routines

#include <psx.h>
#include <stdio.h>
#include <strings.h>
#include "font.h"
#include "costbl.h"

unsigned int *linked_list;
int linked_list_pos;

int fb_font_x, fb_font_y, fb_font_cx, fb_font_cy;

#define GPU_DATA_PORT_ADDR			0x1f801810
#define GPU_CONTROL_PORT_ADDR			0x1f801814
#define GPU_DATA_PORT				*((unsigned int*)GPU_DATA_PORT_ADDR)
#define GPU_CONTROL_PORT			*((unsigned int*)GPU_CONTROL_PORT_ADDR) 

#define DPCR					*((unsigned int*)0x1f8010f0)
#define D2_MADR					*((unsigned int*)0x1f8010a0)
#define D2_BCR					*((unsigned int*)0x1f8010a4)
#define D2_CHCR					*((unsigned int*)0x1f8010a8)

#define get_clutid(cx, cy)			(((cx&0x3ff)>>4)|((cy&0x1ff)<<6))

unsigned int prfont_flags = 0;
int prfont_scale_x = 0;
int prfont_scale_y = 0;

unsigned short GsScreenW;
unsigned short GsScreenH;

unsigned short GsCurDrawEnvW;
unsigned short GsCurDrawEnvH;

double gs_vbuf[4][3];

unsigned int PRFONT_SCALEX(int i)
{
	prfont_scale_x = i;
	return 0;
}

unsigned int PRFONT_SCALEY(int i)
{
	prfont_scale_y = i;
	return 0;
}

unsigned int draw_mode_packet;

unsigned int setup_attribs(unsigned char tpage, unsigned int attribute, unsigned char *packet);
void gs_internal_vector_rotate(int x_a, int y_a, int z_a, double *v, double *n);

unsigned char gpu_stringbuf[512];

int gs_calculate_scaled_size(int size, int scale)
{
	if(scale > 8)
		return (size * scale) / SCALE_ONE;
	else if(scale == 0)
		return size;
	else if(scale > 0)
		return size * scale;
	else if(scale > -8)
		return size / (scale * -1);
	
	return (size * SCALE_ONE) / -scale;
}		
	
void GsSetList(unsigned int *listptr)
{
	linked_list = listptr;
	linked_list_pos = 0;
}

void GsDrawList()
{
	int x = 0;

	/* Put a terminator, so the link listed ends. */
	linked_list[linked_list_pos] = 0x00ffffff;

//	do{printf("linked_list[%d] = %08x\n", x, linked_list[x]);}while(linked_list[x++]!=0xffffff);

	#warning "Let's hope this works well."

	while(!(GPU_CONTROL_PORT & (1<<0x1a))); /* Wait for the GPU to finish
						 * drawing primitives. */
	while(!(GPU_CONTROL_PORT & (1<<0x1c))); /* Wait for the GPU to be free */
	
	gpu_ctrl(4, 2); // DMA CPU->GPU mode
	D2_MADR = (unsigned int)linked_list;
	D2_BCR = 0;
	D2_CHCR = (1<<0xa)|1|(1<<0x18);
	
	linked_list_pos = 0;
}

void GsSortPoly3(GsPoly3 *poly3)
{
	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x20;
	unsigned int md;
	
	md = setup_attribs(0, poly3->attribute, &pkt);
	
	linked_list[linked_list_pos++] = 0x05000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(poly3->b<<16)|(poly3->g<<8)|(poly3->r);
	
	for(x = 0; x < 3; x++)
		linked_list[linked_list_pos++] = ((poly3->y[x]&0x7ff)<<16)|(poly3->x[x]&0x7ff);
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}
	
void GsSortPoly4(GsPoly4 *poly4)
{
	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x28;
	unsigned int md;
	
	md = setup_attribs(0, poly4->attribute, &pkt);
	
	linked_list[linked_list_pos++] = 0x06000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(poly4->b<<16)|(poly4->g<<8)|(poly4->r);
	
	for(x = 0; x < 4; x++)
		linked_list[linked_list_pos++] = ((poly4->y[x]&0x7ff)<<16)|(poly4->x[x]&0x7ff);
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortGPoly3(GsGPoly3 *poly3)
{
	// PKT 0x30

	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x30;
	unsigned int md;
	
	md = setup_attribs(0, poly3->attribute, &pkt);
	
	linked_list[linked_list_pos++] = 0x07000000;
	linked_list[linked_list_pos++] = md;
	
	for(x = 0; x < 3; x++)
	{
		linked_list[linked_list_pos++] = (poly3->b[x]<<16)|(poly3->g[x]<<8)|(poly3->r[x]) | ((x == 0)?(pkt<<24):0);
		linked_list[linked_list_pos++] = ((poly3->y[x]&0x7ff)<<16)|(poly3->x[x]&0x7ff);
	}
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortGPoly4(GsGPoly4 *poly4)
{
	// PKT 0x38

	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x38;
	unsigned int md;
	
	md = setup_attribs(0, poly4->attribute, &pkt);	
	
	linked_list[linked_list_pos++] = 0x09000000;
	linked_list[linked_list_pos++] = md;
	
	for(x = 0; x < 4; x++)
	{
		linked_list[linked_list_pos++] = (poly4->b[x]<<16)|(poly4->g[x]<<8)|(poly4->r[x]) | ((x == 0)?(pkt<<24):0);
		linked_list[linked_list_pos++] = ((poly4->y[x]&0x7ff)<<16)|(poly4->x[x]&0x7ff);
	}
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortLine(GsLine *line)
{
	// PKT 0x40
	
	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x40;
	unsigned int md;
	
	md = setup_attribs(0, line->attribute, &pkt);	
	
	linked_list[linked_list_pos++] = 0x04000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(line->b<<16)|(line->g<<8)|(line->r);
	
	for(x = 0; x < 2; x++)
		linked_list[linked_list_pos++] = ((line->y[x]&0x7ff)<<16)|(line->x[x]&0x7ff);
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortGLine(GsGLine *line)
{
	// PKT 0x50

	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x50;
	unsigned int md;
	
	md = setup_attribs(0, line->attribute, &pkt);	
	
	linked_list[linked_list_pos++] = 0x05000000;
	linked_list[linked_list_pos++] = md;
	for(x=0;x<2;x++)
	{
		linked_list[linked_list_pos++] = (line->b[x]<<16)|(line->g[x]<<8)|(line->r[x])|((x == 0)?(pkt<<24):0);
		linked_list[linked_list_pos++] = ((line->y[x]&0x7ff)<<16)|(line->x[x] & 0x7ff);
	}
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortDot(GsDot *dot)
{
	// PKT 0x68

	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x68;
	unsigned int md;
	
	md = setup_attribs(0, dot->attribute, &pkt);	
	
	linked_list[linked_list_pos++] = 0x03000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(dot->b<<16)|(dot->g<<8)|(dot->r);
	linked_list[linked_list_pos++] = ((dot->y&0x7ff)<<16)|(dot->x&0x7ff);
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortSprite(GsSprite *sprite)
{
	GsTPoly4 tpoly4;
	int x, y;
	int sx = sprite->x & 0x7ff;
	int sy = sprite->y & 0x7ff;
	int mcx, mcy;

	/*if(sprite->w > 256)
		sprite->w = 256;
		
	if(sprite->h > 256)
		sprite->h = 256;*/

	// If "sprite" has no flipping and no scaling use sprite primitive
	// otherwise manipulate a 4 point textured polygon primitive
	
	if(sprite->rotate != 0)
	{
		tpoly4.u[0] = sprite->u;
		tpoly4.v[0] = sprite->v;

		tpoly4.u[1] = sprite->u;
		tpoly4.v[1] = sprite->v + sprite->h;

		tpoly4.u[2] = sprite->u + sprite->w;
		tpoly4.v[2] = sprite->v;

		tpoly4.u[3] = sprite->u + sprite->w;
		tpoly4.v[3] = sprite->v + sprite->h;

		gs_vbuf[0][2] = gs_vbuf[1][2] = gs_vbuf[2][2] = gs_vbuf[3][2] = 0;

		mcx = sprite->mx + sprite->x;
		mcy = sprite->my + sprite->y;

		gs_vbuf[0][0] = -(mcx - sprite->x);
		gs_vbuf[0][1] = (mcy - sprite->y);

		gs_vbuf[1][0] = -(mcx - sprite->x);
		gs_vbuf[1][1] = (mcy - (sprite->y + gs_calculate_scaled_size(sprite->h, sprite->scaley)));

		gs_vbuf[2][0] =  -(mcx - (sprite->x + gs_calculate_scaled_size(sprite->w, sprite->scalex)));
		gs_vbuf[2][1] = (mcy - sprite->y);

		gs_vbuf[3][0] =  -(mcx - (sprite->x + gs_calculate_scaled_size(sprite->w, sprite->scalex)));
		gs_vbuf[3][1] = (mcy - (sprite->y + gs_calculate_scaled_size(sprite->h, sprite->scaley)));

		for(x = 0; x < 4; x++)
		{
			gs_internal_vector_rotate(0, 0, sprite->rotate, gs_vbuf[x], gs_vbuf[x]);
			tpoly4.x[x] = mcx + gs_vbuf[x][0]; 
			tpoly4.y[x] = mcy + gs_vbuf[x][1];
		}

		tpoly4.r = sprite->r;
		tpoly4.g = sprite->g;
		tpoly4.b = sprite->b;
		tpoly4.attribute = sprite->attribute;
		tpoly4.tpage = sprite->tpage;
		tpoly4.cx = sprite->cx;
		tpoly4.cy = sprite->cy;

		GsSortTPoly4(&tpoly4);
	}
	else if((sprite->attribute & (H_FLIP|V_FLIP)) || 
		sprite->scalex != 0 || sprite->scaley != 0)
	{
		x = sprite->w;
		if(x>256)x=256;
		
		y = sprite->h;
		if(y>256)y=256;
		
		if(sprite->scalex > 8)
		{
			x *= sprite->scalex;
			x /= 4096;
		}
		else
		{
			if(sprite->scalex >= 2)
				x*=sprite->scalex;
			else if(sprite->scalex <= -2)
				x/=-sprite->scalex;
		}
		
		if(sprite->scaley > 8)
		{
			y *= sprite->scaley;
			y /= 4096;
		}
		else
		{
			if(sprite->scaley >= 2)
				y*=sprite->scaley;
			else if(sprite->scaley <= -2)
				y/=-sprite->scaley;
		}
			
		tpoly4.x[0] = tpoly4.x[1] = sx;
		tpoly4.x[2] = tpoly4.x[3] = (sx + x);
		tpoly4.y[0] = tpoly4.y[2] = sy;
		tpoly4.y[1] = tpoly4.y[3] = (sy + y);
		
		if(sprite->attribute &  H_FLIP)
		{
			tpoly4.u[0] = tpoly4.u[1] = (sprite->u + sprite->w) - 1;
			tpoly4.u[2] = tpoly4.u[3] = sprite->u;
		}
		else
		{
			tpoly4.u[0] = tpoly4.u[1] = sprite->u;
			tpoly4.u[2] = tpoly4.u[3] = (sprite->u + sprite->w);
		}	
					
		if(sprite->attribute & V_FLIP)
		{
			tpoly4.v[0] = tpoly4.v[2] = (sprite->v + sprite->h) - 1;
			tpoly4.v[1] = tpoly4.v[3] = sprite->v;
		}
		else
		{
			tpoly4.v[0] = tpoly4.v[2] = sprite->v;
			tpoly4.v[1] = tpoly4.v[3] = (sprite->v + sprite->h);
		}

		tpoly4.r = sprite->r;
		tpoly4.g = sprite->g;
		tpoly4.b = sprite->b;
		tpoly4.attribute = sprite->attribute;
		tpoly4.tpage = sprite->tpage;
		tpoly4.cx = sprite->cx;
		tpoly4.cy = sprite->cy;
		
		GsSortTPoly4(&tpoly4);
	}
	else
	{
		GsSortSimpleSprite(sprite);
	}
}

void GsSortSimpleSprite(GsSprite *sprite)
{
	unsigned int orig_pos = linked_list_pos;
	unsigned char pkt = 0x64;
	unsigned int md;

	md = setup_attribs(sprite->tpage, sprite->attribute, &pkt);

	linked_list[linked_list_pos++] = 0x05000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(sprite->b<<16)|(sprite->g<<8)|sprite->r;
	linked_list[linked_list_pos++] = ((sprite->y&0x7ff)<<16)|(sprite->x&0x7ff);
	linked_list[linked_list_pos++] = (get_clutid(sprite->cx,sprite->cy)<<16)|(sprite->v<<8)|sprite->u;
	linked_list[linked_list_pos++] = (sprite->h<<16)|sprite->w;
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortRectangle(GsRectangle *rectangle)
{
	unsigned int orig_pos = linked_list_pos;
	unsigned char pkt = 0x60;
	unsigned int md;
	
	md = setup_attribs(0, rectangle->attribute, &pkt);

	linked_list[linked_list_pos++] = 0x04000000;
	linked_list[linked_list_pos++] = md;
	linked_list[linked_list_pos++] = (pkt<<24)|(rectangle->b<<16)|(rectangle->g<<8)|(rectangle->r);
	linked_list[linked_list_pos++] = ((rectangle->y&0x7ff)<<16)|(rectangle->x&0x7ff);
	linked_list[linked_list_pos++] = (rectangle->h<<16)|rectangle->w;
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortTPoly4(GsTPoly4 *tpoly4)
{
	unsigned int orig_pos = linked_list_pos;
	unsigned char pkt = 0x2c;
	unsigned int md;
	
	/*md = setup_attribs(tpoly4->tpage, tpoly4->attribute, &pkt);*/
	
	//printf("tpoly4->tpage = %d\n", tpoly4->tpage);
	
	md = setup_attribs(tpoly4->tpage, tpoly4->attribute, &pkt);

	//printf("pkt = %x\n", pkt);

	linked_list[linked_list_pos++] = 0x09000000;
	//linked_list[linked_list_pos++] = md;
	//linked_list[linked_list_pos++] = 0xe0000000;
	//linked_list[linked_list_pos++] = 0xe1000105;
	
	//printf("tpoly4 md: %08x\n", md);
	linked_list[linked_list_pos++] = (pkt<<24)|(tpoly4->b<<16)|(tpoly4->g<<8)|(tpoly4->r);
	linked_list[linked_list_pos++] = ((tpoly4->y[0]&0x7ff)<<16)|(tpoly4->x[0]&0x7ff);
	linked_list[linked_list_pos++] = (get_clutid(tpoly4->cx, tpoly4->cy)<<16)|(tpoly4->v[0]<<8)|tpoly4->u[0];
	linked_list[linked_list_pos++] = ((tpoly4->y[1]&0x7ff)<<16)|(tpoly4->x[1]&0x7ff);
	linked_list[linked_list_pos++] = (md << 16)|(tpoly4->v[1]<<8)|tpoly4->u[1];
	linked_list[linked_list_pos++] = ((tpoly4->y[2]&0x7ff)<<16)|(tpoly4->x[2]&0x7ff);
	linked_list[linked_list_pos++] = (tpoly4->v[2]<<8)|tpoly4->u[2];
	linked_list[linked_list_pos++] = ((tpoly4->y[3]&0x7ff)<<16)|(tpoly4->x[3]&0x7ff);
	linked_list[linked_list_pos++] = (tpoly4->v[3]<<8)|tpoly4->u[3];
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void GsSortTPoly3(GsTPoly3 *tpoly3)
{
	int orig_pos = linked_list_pos;
	int x;
	unsigned char pkt = 0x24;
	unsigned int md;
	
	md = setup_attribs(tpoly3->tpage, tpoly3->attribute, &pkt);
	
	linked_list[linked_list_pos++] = 0x07000000;
	linked_list[linked_list_pos++] =
		(pkt<<24)|(tpoly3->b<<16)|(tpoly3->g<<8)|(tpoly3->r);
	
	for(x = 0; x < 3; x++)
	{
		linked_list[linked_list_pos++] = ((tpoly3->y[x]&0x7ff)<<16)|(tpoly3->x[x]&0x7ff);
		linked_list[linked_list_pos] = (tpoly3->u[x]<<8)|tpoly3->v[x];
		
		switch(x)
		{
			case 0:
				linked_list[linked_list_pos++] |= 
					get_clutid(tpoly3->cx, tpoly3->cy) << 16;
			break;
			case 1:
				linked_list[linked_list_pos++] |=
					md << 16;
			break;
		}
	}		
	
	linked_list[orig_pos] |= ((unsigned int)&linked_list[linked_list_pos]) & 0xffffff;
}

void MoveImage(int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
	/*
	 * This seems more like "CopyImage"...
	 */

	while(!(GPU_CONTROL_PORT & (1<<0x1c)));
	
	GPU_CONTROL_PORT = 0x04000000;
	GPU_DATA_PORT = 0x01000000; // Reset command buffer
	GPU_DATA_PORT = 0xE6000000;
	GPU_DATA_PORT = 0x80000000;
	GPU_DATA_PORT = (src_y<<16)|src_x;
	GPU_DATA_PORT = (dst_y<<16)|dst_x;
	GPU_DATA_PORT = (h<<16)|w;
}

/*
 * Add a method to add arbitrary data to the packet list
 */	
			
void LoadImage(void *img, int x, int y, int w, int h)
{
	unsigned short *image = (unsigned short*)img;
	int a, l;
	
	//printf("LoadImage: %d, %d, %d, %d\n", x, y, w, h);
	
	while(!(GPU_CONTROL_PORT & (1<<0x1c)));
		
	GPU_CONTROL_PORT = 0x04000000; // Disable DMA
	
// Reset should be on data port ! otherwise we won't be able
// to write CLUTs for some time after they've been used!	
// (why??)	
	
	GPU_DATA_PORT = 0x01000000;
	GPU_DATA_PORT = 0xE6000000; // disable masking stuff !!
	GPU_DATA_PORT = 0xA0000000;
	GPU_DATA_PORT = (y<<16)|x;
	GPU_DATA_PORT = (h<<16)|w;
	
	l = w*h;
	if(l&1)l++;
	
	for(a = 0; a < l; a+=2)
		GPU_DATA_PORT = image[a]|(image[a+1]<<16);
		
	GPU_CONTROL_PORT = 0x01000000;
//	while(!(GPU_CONTROL_PORT & (1<<0x1c)));
}

/*void LoadImage(void *img, int x, int y, int w, int h)
{
	GPU_dw(x, y, w, h, img);
	
	/*int l;
	
	printf("LoadImage: %d, %d, %d, %d\n", x, y, w, h);
	
	l = w*h;
	if(l&1)l++;
	l/=2;
	
	while(!(GPU_CONTROL_PORT & (1<<0x1c))); // Wait for the GPU to be free 
	
	gpu_ctrl(4, 2); // DMA CPU->GPU mode
	D2_MADR = (unsigned int)img;
	D2_BCR = (l << 16) | 1;
	D2_CHCR = 0x01000201;
	
	// Wait for DMA to finish
	
	while(D2_CHCR & (1<<0x18));*/
//}*/

void GsSetDrawEnv(GsDrawEnv *drawenv)
{
	int end_y, end_x;
	int mf;

	/*
	 * Store the 0xe1 packet - we need it because we have to
	 * modify drawing environment for sprites
	 */

	draw_mode_packet = (0xe1<<24)|(drawenv->draw_on_display>=1)<<10|
		(drawenv->dither>=1)<<9;	

	gpu_data_ctrl(0xe1, draw_mode_packet);	
	gpu_data_ctrl(0xe2, 0);
	gpu_data_ctrl(0xe3, (drawenv->y<<10)|drawenv->x);
	
	end_x = (drawenv->x + drawenv->w)-1;
	end_y = (drawenv->y + drawenv->h)-1;
	
	gpu_data_ctrl(0xe4, (end_y<<10)|end_x);
	
	#warning "Check drawing offset better."
	gpu_data_ctrl(0xe5, (drawenv->y<<11)|drawenv->x);
	//gpu_data_ctrl(0xe5, 0);
	
	
	mf = 0;
	if(drawenv->set_mask) mf|=MASK_SET;
	if(drawenv->ignore_mask) mf|=MASK_IGNORE;
	
	GsSetMasking(mf);

	GsCurDrawEnvW = drawenv->w;
	GsCurDrawEnvH = drawenv->h;
}

void GsSetDispEnv(GsDispEnv *dispenv)
{
	gpu_ctrl(5, (dispenv->y<<10)|dispenv->x); // Display offset
}

void gpu_ctrl(unsigned int command, unsigned int param)
{
	unsigned int doubleword = (command << 0x18) | param;
	
	GPU_CONTROL_PORT = 0x01000000;
	GPU_CONTROL_PORT = doubleword;
}

void gpu_data(unsigned int data)
{
	GPU_DATA_PORT = data;
}

void gpu_data_ctrl(unsigned int command, unsigned int param)
{
	unsigned int doubleword = (command << 0x18) | param;
	
	GPU_CONTROL_PORT = 0x01000000;
	GPU_DATA_PORT = doubleword;
}

unsigned int setup_attribs(unsigned char tpage, unsigned int attribute, unsigned char *packet)
{
	unsigned int orig_pos = linked_list_pos;
	unsigned int sprite_mode_packet;

	//printf("tpage = %d, attribute = %x, packet = %x\n", tpage, attribute, packet);
	//while(1);*/
	
/*
 * First, setup draw mode setting.
 */

	sprite_mode_packet = draw_mode_packet;
	sprite_mode_packet|= tpage & 0x1f; /* Texture page */
	sprite_mode_packet|= (attribute & 3) << 7; /* Color mode */
	sprite_mode_packet|= ((attribute>>2)&3) << 5; /* Translucency mode */

/*
 * Check for STP bit flag in attribute, and modify packet byte accordingly
 */
	if(attribute & 16)
		*packet|=2;

	//printf("sprite_mode_packet = %08x\n", sprite_mode_packet);
		
	return sprite_mode_packet;
}

unsigned int GsListPos()
{
	return linked_list_pos;
}

	
	
void GsEnableDisplay(int enable)
{
	gpu_ctrl(3, enable ? 0 : 1);
}

void GsReset()
{
	gpu_ctrl(0, 0); // Reset GPU
}

void GsInit()
{
	//gpu_ctrl(0, 0); // Reset GPU
	GsReset(); // Reset GPU
	
	DPCR |= (1<<0xb); // Enable dma channel 2
	gpu_ctrl(4, 2); // DMA CPU->GPU mode
	
	//gpu_ctrl(3, 1); // Disable display
	GsEnableDisplay(0); // Disable display
	/*gpu_ctrl(6, 0xc40240); // Horizontal start end
	gpu_ctrl(7, 0x049025); // Vertical start end*/
	//DrawFBRect(0, 0, 1023, 511, 0, 0, 0);
}

/*void SetVBlankHandler2(void *(callback)())
{
	unsigned int eventid;

	EnterCriticalSection();

	IMASK|=8; // Enable VBLANK interrupt
	eventid = openevent(0xf0000001, 2, 0x1000, vblank_handler);
	
	if(eventid == -1)
	{
		printf("SetVBlankHandler: Failed to open event!\n");
		return;
	}
	else
		printf("SetVBlankHandler: Event opened successfully!\n");
	
	if(enableevent(eventid) == 0)
	{
		printf("SetVBlankHandler: Failed to enable event!\n");
		// Shouldn't we close the event as well?
		return;
	}
	else
		printf("SetVBlankHandler: Event enabled successfully!\n");
		
	ExitCriticalSection();
}*/

int GsSetVideoMode(int width, int height, int video_mode)
{
	// Just a quick wrapper for GsSetVideoModeEx
	return GsSetVideoModeEx(width, height, video_mode, 0, 0, 0);
}

int GsSetVideoModeEx(int width, int height, int video_mode, int rgb24,
	int inter, int reverse)
{
	unsigned char mode = 0;
	
	GsEnableDisplay(0);
	
	if(video_mode == VMODE_NTSC)
	{
		gpu_ctrl(6, 0xC4E24E); // Horizontal screen range
		gpu_ctrl(7, 0x040010); // Vertical screen range
	}
	else
	{
		gpu_ctrl(6, 0xC62262); // Horizontal screen range
		gpu_ctrl(7, 0x04B42D); // Vertical screen range
	}
		
	switch(height)
	{
		case 240:
		break;
		case 480:
			mode|=4;
		break;
		default:
			printf("%s: error, unknown width %d!\n", __FUNCTION__, width);
			return 0;
	}
	
	switch(width)
	{
		case 256:
		break;
		case 320:
			mode|=1;
		break;
		case 512:
			mode|=2;
		break;
		case 640:
			mode|=3;
		break;
		case 384:
			mode|=64;
		break;
		default:
			printf("%s: error, unknown height %d!\n", __FUNCTION__, height);
			return 0;
	}
	
	if(video_mode)mode|=8; // Set PAL
	if(rgb24)mode|=16; // Set unaccellerated 24-bit mode
	if(inter)mode|=32; // Set interlaced video mode
	if(reverse)mode|=128; // Set reverse flag (?)
	
	gpu_ctrl(8, mode);
	GsEnableDisplay(1);
	
	GsScreenW = width;
	GsScreenH = height;
	
	return 1;
}

void DrawFBRect(int x, int y, int w, int h, int r, int g, int b)
{
	while(!(GPU_CONTROL_PORT & (1<<0x1c)));

	// Disable DMA
	GPU_CONTROL_PORT = 0x04000000;
	
	GPU_DATA_PORT = 0x01000000; // Reset data port
	GPU_DATA_PORT = 0xE6000000; // Disable masking stuff
	gpu_data_ctrl(2, ((b&0xff)<<16)|((g&0xff)<<8)|r);
	GPU_DATA_PORT = (y<<16)|x;
	GPU_DATA_PORT = (h<<16)|w;
}

void GsClearMem()
{
	// "Clears" the entire video memory by using DrawFBRect
	// and waits that it has finished drawing...
	
	DrawFBRect(0,0,1023,511,0,0,0);
	while(GsIsDrawing());
	DrawFBRect(0,511,1023,1,0,0,0);
	while(GsIsDrawing());
	DrawFBRect(1023,511,1,1,0,0,0);
	while(GsIsDrawing());
}

int GsImageFromTim(GsImage *image, void *timdata)
{
	unsigned int *timdata_i = (unsigned int*)timdata;
	unsigned short *timdata_s = (unsigned short*)timdata;
	unsigned int pdata_pos;
	unsigned int pdata_pos_s;
	
	//printf("timdata_i[0] = %08x\n", timdata_i[0]);
	
	if(timdata_i[0] != 0x10)
	{
		//printf("timdata_i[0] = %08x\n", timdata_i[0]);
		return 0; // Unknown version or ID
	}
		
	image->pmode = timdata_i[1] & 7;
	
	//printf("image->pmode = %d\n", image->pmode);
	
	image->has_clut = (timdata_i[1] & 8) ? 1 : 0;
	
	if(!image->has_clut)
		pdata_pos = 8;
	else
	{
		pdata_pos = 8 + timdata_i[2];
		image->clut_x = timdata_s[6];
		image->clut_y = timdata_s[7];
		image->clut_w = timdata_s[8];
		image->clut_h = timdata_s[9];
		image->clut_data = &timdata_s[10];
		
		/*printf("image->clut_y = %d\n", image->clut_y);
		printf("image->clut_x = %d\n", image->clut_x);
		printf("image->clut_h = %d\n", image->clut_h);
		printf("image->clut_w = %d\n", image->clut_w);*/
	}
	
	pdata_pos_s = pdata_pos / 2;
	
	image->x = timdata_s[pdata_pos_s + 2];
	image->y = timdata_s[pdata_pos_s + 3];
	image->w = timdata_s[pdata_pos_s + 4];
	image->h = timdata_s[pdata_pos_s + 5];
	image->data = &timdata_s[pdata_pos_s + 6];
		
	/*printf("image->y = %d\n", image->y);
	printf("image->x = %d\n", image->x);
	printf("image->h = %d\n", image->h);
	printf("image->w = %d\n", image->w);*/

	return 1;
}

int GsUploadImage(GsImage *image)
{
	if(image->has_clut)
		LoadImage(image->clut_data, image->clut_x, image->clut_y,
			image->clut_w, image->clut_h);
			
	LoadImage(image->data, image->x, image->y, image->w, image->h);
}

int GsSpriteFromImage(GsSprite *sprite, GsImage *image, int do_upload)
{
	if(do_upload)
		GsUploadImage(image);
		
	bzero(sprite, sizeof(GsSprite));
	
	sprite->tpage = (image->x / 64) + ((image->y/256)*16);
	sprite->u = image->x & 0x3f;
	sprite->v = image->y & 0xff;
	
	sprite->cx = image->clut_x;
	sprite->cy = image->clut_y;
	
	if(image->pmode == 0) // 4-bit pixel mode
		sprite->u*=4;
	else if(image->pmode == 1) // 8-bit pixel mode
		sprite->u*=2;
	
	switch(image->pmode)
	{
		case 0:
			sprite->w = image->w * 4;
		break;
		case 1:
			sprite->w = image->w * 2;
		break;
		case 2:
			sprite->w = image->w;
		break;
		case 3:
			sprite->w = image->w + (image->w / 2);
		break;
	}
	
	sprite->h = image->h;
	sprite->attribute = COLORMODE(image->pmode);
	sprite->r = sprite->g = sprite->b = NORMAL_LUMINOSITY;
	
	return 1;
}

void GsSetMasking(unsigned char flag)
{
	gpu_data_ctrl(0xe6, flag);
}

int GsIsDrawing()
{
	if (!(GPU_CONTROL_PORT & (1<<0x1a)))
		return 1;
	else
		return 0;
}



// Functions which use default values to use when you do not
// really need to fiddle with all the fields of the structure

void GsSetDrawEnvSimple(int x, int y, int w, int h)
{
	GsDrawEnv env;
	
	env.dither = 0;
	env.draw_on_display = 1;
	env.x = x;
	env.y = y;
	env.w = w;
	env.h = h;
	env.ignore_mask = 0;
	env.set_mask = 0;
	
	GsSetDrawEnv(&env);
}

void GsSetDispEnvSimple(int x, int y)
{
	GsDispEnv env;
	
	env.x = x;
	env.y = y;
	
	GsSetDispEnv(&env);
}

// Built-in font functions.

void GsLoadFont(int fb_x, int fb_y, int cx, int cy)
{
	unsigned short pal[2] = {0x0, 0x7fff};
	
	LoadImage(psxsdk_font_data, fb_x, fb_y, 16, 128);
	while(GsIsDrawing());
	
	if(cx != -1 && cy != -1)
	{
		LoadImage(pal, cx, cy, 16, 1);
		
		fb_font_cx = cx;
		fb_font_cy = cy;
		
		while(GsIsDrawing());
	}
	
	fb_font_x = fb_x;
	fb_font_y = fb_y;
}

unsigned int GsPrintFont_Draw(int x, int y, int scalex, int scaley)
{
	//int r;
	GsSprite spr;
	char *string;
	int fw, fh;

	/*va_list ap;

	va_start(ap, fmt);*/

//	r = vsnprintf(gpu_stringbuf, 512, fmt, ap);
	
//	va_end(ap);
	fw = gs_calculate_scaled_size(8, scalex);//(8*scalex)/4096;
	fh = gs_calculate_scaled_size(8, scaley);//(8*scaley)/4096;
	
	spr.x = x;
	spr.y = y;
	spr.r = spr.g = spr.b = NORMAL_LUMINOSITY;
	spr.attribute = 0;
	spr.cx = fb_font_cx;
	spr.cy = fb_font_cy;
	spr.tpage = (fb_font_x / 64) + ((fb_font_y / 256)*16);
	spr.w = 8;
	spr.h = 8;
	spr.scalex = scalex;
	spr.scaley = scaley;
	
	string = gpu_stringbuf;
	
	while(*string)
	{
		if(prfont_flags & PRFONT_WRAP)
		{
			if(spr.x >= GsScreenW)
			{
				spr.x = 0;
				spr.y += fh;
			}
		}
		
		if(*string >= ' ' && *string <= '~')
		{
			spr.u = ((fb_font_x & 0x3f)*4)+((*string & 7) << 3);
			spr.v = (fb_font_y & 0xff)+(*string & 0xf8);
			
			if((scalex == 0 || scalex == 1) && (scaley == 0 || scaley == 1))
				GsSortSimpleSprite(&spr);
			else
				GsSortSprite(&spr);
			
			spr.x += fw;
		}
		
		if(*string == '\n')
		{
			spr.x = x;
			spr.y += fh;
		}
		
		string++;
	}
	
	return (spr.y << 16) | spr.x;
}

unsigned int GsPrintFont(int x, int y, char *fmt, ...)
{
	int r;
	GsSprite spr;
	char *string;
	va_list ap;
	int fw = gs_calculate_scaled_size(8, prfont_scale_x);

	va_start(ap, fmt);

	r = vsnprintf(gpu_stringbuf, 512, fmt, ap);
	
	if(prfont_flags & PRFONT_WRAP)
		r = GsPrintFont_Draw(x, y, prfont_scale_x, prfont_scale_y);
	else if(prfont_flags & PRFONT_CENTER)
		r = GsPrintFont_Draw(x - (r * (fw/2)), y, prfont_scale_x, prfont_scale_y);
	else if(prfont_flags & PRFONT_RIGHT)
		r = GsPrintFont_Draw(x - (r * fw), y, prfont_scale_x, prfont_scale_y);
	else
		r = GsPrintFont_Draw(x, y, prfont_scale_x, prfont_scale_y);

	va_end(ap);
	
	return r;
}

void GsSetFont(int fb_x, int fb_y, int cx, int cy)
{
	fb_font_x = fb_x;
	fb_font_y = fb_y;
	fb_font_cx = cx;
	fb_font_cy = cy;
}

void GsSetFontAttrib(unsigned int flags)
{
	prfont_flags = flags;
	
	if(prfont_flags == 0)
	{
		PRFONT_SCALEX(0);
		PRFONT_SCALEY(0);
	}
}

double gs_internal_cos(int a)
{
	int a_a = (a>>12)-(((a>>12)/360)*360);

	if(a_a>=0 && a_a<=90)
		return gs_rot_cos_tbl[a_a];
	else if(a_a>90 && a_a<=180)
		return -gs_rot_cos_tbl[180 - a_a];
	else if(a_a>180 && a_a<=270)
		return -gs_rot_cos_tbl[a_a - 180];
	else if(a_a>270 && a_a<=359)
		return gs_rot_cos_tbl[360 - a_a];

	return 0;
}

double gs_internal_sin(int a)
{
	int a_a = (a>>12)-(((a>>12)/360)*360);

	if(a_a>=0 && a_a<=90)
		return gs_rot_cos_tbl[90-a_a];
	else if(a_a>90 && a_a<=180)
		return gs_rot_cos_tbl[a_a-90];
	else if(a_a>180 && a_a<=270)
		return -gs_rot_cos_tbl[270-a_a];
	else if(a_a>270 && a_a<=359)
		return -gs_rot_cos_tbl[a_a-270];

	return 0;
}

void gs_internal_vector_rotate(int x_a, int y_a, int z_a, double *v, double *n)
{
// This function can rotate a vector about the X, Y and Z axes
// If you don't want to rotate a vector about an axis, pass 0 as angle to that axis
// It is correct to pass the same argument to v and n, as calculations are first done in an internal buffer
// and then stored in the output array

	double axis_m[3][3];
	double b[3];
	double k[3], s[3];
	int x;

	k[0] = gs_internal_cos(x_a);
	k[1] = gs_internal_cos(y_a);
	k[2] = gs_internal_cos(z_a);

	s[0] = gs_internal_sin(x_a);
	s[1] = gs_internal_sin(y_a);
	s[2] = gs_internal_sin(z_a);

	axis_m[0][0] = k[1] * k[2];
	axis_m[0][1] = (k[0] * s[2]) + (s[0]*s[1]*k[2]);
	axis_m[0][2] = (s[0]*s[2]) - (k[0]*s[1]*k[2]);
	axis_m[1][0] = -(k[1] * s[2]);
	axis_m[1][1] = (k[0]*k[2]) - (s[0]*s[1]*s[2]);
	axis_m[1][2] = (s[0]*k[2]) + (k[0]*s[1]*s[2]);
	axis_m[2][0] = s[1];
	axis_m[2][1] = -(s[0]*k[1]);
	axis_m[2][2] = k[0]*k[1];

	for(x=0;x<3;x++)
		b[x] = (axis_m[x][0] * v[0]) + (axis_m[x][1] * v[1]) + (axis_m[x][2] * v[2]);

	b[1]=-b[1];

	for(x=0;x<3;x++)
		n[x]=b[x];
}

int GsIsWorking()
{
	return GsIsDrawing();
}

void GsSortCls(int r, int g, int b)
{
	GsRectangle rect;

	rect.r = r;
	rect.g = g;
	rect.b = b;
	rect.x = 0;
	rect.y = 0;
	rect.attribute = 0;
	rect.w = GsCurDrawEnvW;
	rect.h = GsCurDrawEnvH;

	GsSortRectangle(&rect);
}


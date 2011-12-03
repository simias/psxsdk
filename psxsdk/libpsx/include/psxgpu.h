#ifndef _PSXGPU_H
#define _PSXGPU_H

/*
 * GsInit() initializes the GPU.
 * The display is left disabled.
 * You can enable it with GsEnableDisplay() or more preferably with GsSetVideoMode()
 */
 
void GsInit();

/*
 * GsReset() resets the GPU
 */
 
void GsReset();

/*
 * GsEnableDisplay() enables the display if enable is TRUE (>=1) or disables it if it is FALSE (=0)
 */
 
void GsEnableDisplay(int enable);

/*
 * GsSetVideoMode() sets a video mode. It does so in a quicker way
 * than GsSetVideoModeEx(), which wants more arguments but offers greater control
 */
 
int GsSetVideoMode(int width, int height, int video_mode);

/*
 * GsSetVideoModeEx() works like GsSetVideoMode() but offers more control.
 * Interlaced, 24-bit RGB, and "reverse" flags can be specified.
 */

int GsSetVideoModeEx(int width, int height, int video_mode, int rgb24, int inter, int reverse);

/*
 * GsSetList() assigns the internal pointer to the linked list to the desired one,
 * and resets the linked list position counter.
 * The memory address specified by your pointer has to have enough space free to contain all
 * the packets which you want to send.
 */

void GsSetList(unsigned int *listptr);

/*
 * GsDrawList() draws the linked list
 */
 
void GsDrawList();

typedef struct
{
	/* Monochrome 3 point polygon */

	unsigned char r, g, b; /* Color */
	short x[3], y[3];
	unsigned int attribute; /* Attribute */
}GsPoly3;

typedef struct
{
	/* Monochrome 4 point polygon */

	unsigned char r, g, b; /* Color */
	short x[4], y[4];
	unsigned int attribute; /* Attribute */
}GsPoly4;

typedef struct
{
	/* Textured 3 point polygon */

	unsigned char r, g, b; /* Color */
	short x[3], y[3];
	unsigned char u[3], v[3];
	short cx, cy;
	unsigned int attribute; /* Attribute */
	unsigned char tpage;
}GsTPoly3;

typedef struct
{
	short x[4], y[4];
	unsigned char r, g, b;
	short cx, cy;
	unsigned char tpage;
	unsigned char u[4], v[4];
	unsigned int attribute;
}GsTPoly4;

typedef struct
{
	/* Graduated 3 point polygon */

	unsigned char r[3], g[3], b[3]; /* Color */
	short x[3], y[3];
	unsigned int attribute; /* Attribute */
}GsGPoly3;

typedef struct
{
	/* Graduated 4 point polygon */

	unsigned char r[3], g[3], b[3]; /* Color */
	short x[4], y[4];
	unsigned int attribute; /* Attribute */
}GsGPoly4;

typedef struct
{
	/* Monochrome line */

	unsigned char r, g, b; /* Color */
	short x[2], y[2];
	unsigned int attribute; /* Attribute */
}GsLine;

typedef struct
{
	/* Dot (pixel) */
	unsigned char r, g, b; /* Color */
	short x, y;
	unsigned int attribute; /* Attribute */
}GsDot;

typedef struct
{
	/* Monochrome line */

	unsigned char r[2], g[2], b[2]; /* Color */
	short x[2], y[2];
	unsigned int attribute; /* Attribute */
}GsGLine;

typedef struct
{
	/* Sprite */
	
	short x, y; /* X, Y positions */
	unsigned char u, v; /* Offset into texture page of sprite image data */
	short w, h; /* Width and height of sprite */
	short cx, cy; /* Color look up table (palette) X, Y positions */
	unsigned char r, g, b; /* Luminosity of color components - 128 is normal luminosity */
	unsigned char tpage; /* Texture page */
	unsigned int attribute; /* Attribute */
	
	/* Scaling? These are only candidates...
	
	   scalex:
	    Denotes horizontal scaling
	    
	    0 = true size (unmodified)
	    1 = true size (*1)
	    2 = double size (*2)
	    3 = triple size (*3)
	    ...
	    
	    -1 = true size (/1)
	    -2 = half size (/2)
	    -3 = one-third size (/3)
	    ...
	    
	   scaley:
	    Denotes vertical scaling
	    
	    *** The behaviour below was introduced in PSXSDK 0.5
	    
	    If scalex > 8,
		resulting width will be (original_width * scalex) / 4096
		scalex = 4096 (SCALE_ONE) (original width), scalex = 2048 (half width), etc.
	    If scaley > 8,
	        resulting height will be (original_height * scaley) / 4096
		works like scalex but affects height
	 */
	 int scalex, scaley;

	int rotate; // Rotation angle - Fixed point notation, 4096 = 1 degree
	int mx, my; // Coordinates of rotation center - relative to coordinates of sprite
}GsSprite;

typedef struct
{
	/* Rectangle */
	
	short x, y;
	short w, h;
	unsigned char r, g, b;
	unsigned int attribute; /* Attribute */
}GsRectangle;

enum
{
 COLORMODE_4BPP, COLORMODE_8BPP, COLORMODE_16BPP, COLORMODE_24BPP
};

#define NORMAL_LUMINOSITY	128

#define COLORMODE(x)		x&3
#define TRANS_MODE(x)		((x&3)<<2)
#define ENABLE_TRANS		(1<<4)
#define H_FLIP			(1<<5)
#define V_FLIP			(1<<6)

typedef struct
{
	/* Drawing environment */
	unsigned char dither;
	unsigned char draw_on_display;
	
	/* 
	 * Drawing area position in the framebuffer and its size 
	 */
	short x, y, w, h;

	/*
	 * Drawing offset
	 */
	//short off_x, off_y;
	
	/*
	 * Masking settings (can also be set with GsSetMasking)
	 */
	
	unsigned char ignore_mask; /* Do not draw over pixels which have their mask bit set */
	unsigned char set_mask; /* If this is set, every pixel drawn will have the mask bit set */
}GsDrawEnv;

typedef struct
{
	/* Display environment. */
	short x, y;
}GsDispEnv;

typedef struct
{
	/* Structure which describes a TIM image */
	
	int pmode;
	int has_clut;
	int clut_x, clut_y, clut_w, clut_h;
	int x, y, w, h;
	void *clut_data;
	void *data;
}GsImage;

/*
 * Adds a monochrome 3 point polygon to the packet list
 */

void GsSortPoly3(GsPoly3 *poly3);

/*
 * Adds a monochrome 4 point polygon to the packet list
 */

void GsSortPoly4(GsPoly4 *poly4);

/*
 * Adds a textured 3 point polygon to the packet list
 */
 
void GsSortTPoly3(GsTPoly3 *tpoly3);

/*
 * Adds a textured 4 point polygon to the packet list
 */

void GsSortTPoly4(GsTPoly4 *tpoly4);

/*
 * Adds a gradated 3 point polygon to the packet list
 */

void GsSortGPoly3(GsGPoly3 *poly3);

/*
 * Adds a gradated 4 point polygon to the packet list
 */

void GsSortGPoly4(GsGPoly4 *poly4);

/*
 * Adds a monochrome line to the packet list
 */
 
void GsSortLine(GsLine *line);

/*
 * Adds a gradated line to the packet list
 */
 
void GsSortGLine(GsGLine *line);

/*
 * Adds a dot (pixel) to the packet list
 */
 
void GsSortDot(GsDot *dot);

/*
 * Adds a sprite to the packet list
 */

void GsSortSprite(GsSprite *sprite);

/*
 * Always adds a REAL sprite to the packet list
 *
 + GsSortSprite() on the other hand checks for scaling and flipping, which
 * are not supported by the "sprite" primitive on the PlayStation, but instead
 * are done by using a textured 4-point polygon accordingly.
 * GsSortSprite() only uses the sprite primitive when all the attributes
 * can be done with a "sprite" primitive as well.
 */
 
void GsSortSimpleSprite(GsSprite *sprite);

/*
 * Adds a rectangle to the packet list
 */
 
void GsSortRectangle(GsRectangle *rectangle);

/*
 * Moves image data from a part of the framebuffer to another.
 * Actually it does a copy.
 */

void MoveImage(int src_x, int src_y, int dst_x, int dst_y, int w, int h);

/*
 * Loads image data into framebuffer memory.
 */

void LoadImage(void *img, int x, int y, int w, int h);

/*
 * Draws a rectangle in the framebuffer, without considering drawing
 * and display environments (i.e. it does so in an absolute way)
 */
 
void DrawFBRect(int x, int y, int w, int h, int r, int g, int b);

/*
 * Set drawing environment
 */

void GsSetDrawEnv(GsDrawEnv *drawenv);

/*
 * Set display environment
 */

void GsSetDispEnv(GsDispEnv *dispenv);

/*
 * Sets masking settings
 */

/* If this flag is set, pixels drawn have MSB set */
#define MASK_SET		1	
/* If this flag is set, pixels aren't drawn over pixels with MSB set */
#define MASK_IGNORE		2
 
void GsSetMasking(unsigned char flag);

/*
 * Return pointer position in linked list
 */
 
unsigned int GsListPos();

/*
 * Three functions which send data to the control port and to the data port
 */
void gpu_ctrl(unsigned int command, unsigned int param);
void gpu_data(unsigned int data);
void gpu_data_ctrl(unsigned int command, unsigned int param);

/*
 * Puts information about a TIM image passed in a buffer in a GsImage structure.
 */

int GsImageFromTim(GsImage *image, void *timdata);

/*
 * Uploads an image described by a GsImage structure to video memory.
 */

int GsUploadImage(GsImage *image);

/*
 * Fills a GsSprite structure with information from an image described
 * by a GsImage structure, then optionally uploads data to video memory.
 * Sprite coordinates are set to 0.
 */
 
int GsSpriteFromImage(GsSprite *sprite, GsImage *image, int do_upload);

/*
 * Returns 1 if GPU is drawing.
 */

int GsIsDrawing();
int GsIsWorking(); // Alias of GsIsDrawing()

/*
 * Clear Video RAM
 */
 
void GsClearMem();

/*
 * Loads the built-in 8x8 font in Video RAM at the specified coordinates
 *
 * fb_x = X coordinate in framebuffer
 * fb_y = Y coordinate in framebuffer
 * cx = X coordinate of black/white CLUT in framebuffer
 * cy = Y coordinate of black/white CLUT in framebuffer
 *
 * Specifying cx and cy as -1 will not load the black&white CLUT to 
 * video memory. Use GsSetFont() to specify clut x and clut y in that case.
 *
 * The font occupies a space of 16x128 pixels in 16-bit mode,
 * and 64x128 in 4-bit mode.
 */
 
void GsLoadFont(int fb_x, int fb_y, int cx, int cy);

/*
 * Prints string using 8x8 font at screen coordinates x, y
 */

unsigned int GsPrintFont(int x, int y, char *fmt, ...);

/*
 * Change font coordinates without reloading it
 */
 
void GsSetFont(int fb_x, int fb_y, int cx, int cy);

// This is a function that sets an internal variable
// flags should be specified as an OR mask

// to set the wrap and scale attributes, with X scaling factor 2 and Y scaling
// factor 3, for example:
//	GsSetFontAttrib(PRFONT_WRAP | PRFONT_SCALE
//				| PRFONT_SCALEX(2) | PRFONT_SCALEY(3));
//
// to remove all attributes (normal printing):
//
//	GsSetFontAttrib(0);
//
// PRFONT_WRAP can't coexist with PRFONT_CENTER or PRFONT_RIGHT
// PRFONT_CENTER and PRFONT_RIGHT are justifying attributes and as such
// they are mutually exclusive - they cannot coexist with each other.
// Specifying both will give priority to PRFONT_CENTER.	
//
// Attribute list:
//
// PRFONT_WRAP - Wrap to next row when the text would fall off the screen
//				Ideal for debug
// PRFONT_SCALE - Enable font scaling
// PRFONT_SCALEX(i) - Specifies the factor "i" as the X scaling factor
//                                    This has the same meaning as a sprite's scaling factor
// PRFONT_SCALEY(i) - Specify the factor "i" as the Y scaling factor
//                                    This has the same meaning as a sprite's scaling factor
// PRFONT_RIGHT - Justifies text to the right
// PRFONT_CENTER - Justifies text at the center

void GsSetFontAttrib(unsigned int flags);


/*
 * Simpler functions to set drawing and display environment
 * GsSetDrawEnvSimple enables drawing on the display area by default
 * and all masking stuff is disabled...
 */

void GsSetDrawEnvSimple(int x, int y, int w, int h);
void GsSetDispEnvSimple(int x, int y);

enum{VMODE_NTSC, VMODE_PAL};

// One scaling unit.

#define SCALE_ONE		4096

// One rotation unit.

#define ROTATE_ONE	4096

// GsSetFontAttrib() Attribute Flags

#define PRFONT_WRAP				1
#define PRFONT_CENTER			2
#define PRFONT_RIGHT				4
#define PRFONT_SCALE				8

// These two below are not really attributes... but use them as if they were.

unsigned int PRFONT_SCALEX(int i);
unsigned int PRFONT_SCALEY(int i);

// Use this to get the final X and Y positions from the return value
// of GsPrintFont(). Especially useful after wrapping.

#define prfont_get_fx(i)				((short)(i & 0xffff))
#define prfont_get_fy(i)				((short)((i >> 16) & 0xffff))

// Width and height of the screen in the current video mode

extern unsigned short GsScreenW;
extern unsigned short GsScreenH;


extern unsigned short GsCurDrawEnvW;
extern unsigned short GsCurDrawEnvY;

// Clear the entire drawing area with specified color

void GsSortCls(int r, int g, int b);

#endif

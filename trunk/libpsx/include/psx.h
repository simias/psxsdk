/*
 * PSXSDK Library include
 */
 
#ifndef _PSX_H
#define _PSX_H

/*
 * Include some GCC builtin includes
 */

#ifndef _PSXSDK_WRAPPER

#include <psxbios.h>

#endif

#include <psxgpu.h>
#include <memcard.h>

/* Pad bits defines. */

#define PAD_LEFT	(1<<15)
#define PAD_RIGHT	(1<<13)
#define PAD_UP		(1<<12)
#define PAD_DOWN	(1<<14)
#define PAD_L2		1
#define PAD_R2		(1<<1)
#define PAD_L1		(1<<2)
#define PAD_R1		(1<<3)
#define PAD_TRIANGLE	(1<<4)
#define PAD_CIRCLE	(1<<5)
#define PAD_CROSS	(1<<6)
#define PAD_SQUARE	(1<<7)
#define PAD_SELECT	(1<<8)
#define PAD_START	(1<<11)

enum
{
     PADTYPE_NONE,
     PADTYPE_MOUSE, // Mouse, SCPH-1030 
     PADTYPE_NAMCO16, // 16-Button Analog pad, SLPH-00001 (Namco)
     PADTYPE_KONAMIGUN, // Gun Controller, SLPH-00014 (Konami)
     PADTYPE_16BTN, // 16 Button, SCPH-1080/1150
     PADTYPE_ANALOGJOY, // Analog Joystick SCPH-1110
     PADTYPE_NAMCOGUN, // Gun Controller, SLPH-00034 (Namco)
     PADTYPE_ANALOGPAD, // Analog Controller, SCPH-1150
     PADTYPE_MULTITAP, // Multi Tap, SCPH-1070
};    

// Root counter specifications

#define RCntCNT0			0xf2000000	// Pixel clock
#define RCntCNT1			0xf2000001	// Horizontal sync
#define RCntCNT2			0xf2000002	// System clock / 8
#define RCntCNT3			0xf2000003	// VSync (VBlank)

// Root counter modes

#define RCntIntr        		0x1000       	// Interrupt mode
#define RCntNotar			0x0100       	// Ignore target and count to 65535 (hex: 0xFFFF)
#define RCntStop                    0x0010       	// Timer stop mode
#define RCntSC          		0x0001            // System Clock mode

struct psx_info
{
	struct
	{
		const char *version; // Kernel version
		int year; // Kernel year
		int month; // Kernel month
		int day; // Kernel day
	}kernel;
	
	struct
	{
		int memory; // RAM memory size
	}system;
};

/*
 * PSX_Init(), initialize library
 */
void PSX_Init();

/*
 * PSX_ReadPad(), takes two pointers for 16-bit bitmasks, one for the first player's joypad,
 * and one for the second one. Their bits are updated to show which buttons were pressed.
 */
void PSX_ReadPad(unsigned short *padbuf, unsigned short *padbuf2);

/*
 * PSX_GetPadType() returns the type of the pad specified by pad_num.
 * 0 is first pad and so on.
 */
 
int PSX_GetPadType(unsigned int pad_num);

/*
 * PSX_GetSysInfo() takes a pointer to a struct psx_info structure, and fills it
 * with information about the PlayStation on which the program is running.
 * PS-OS kernel build date and version are reported among other things.
 */
void PSX_GetSysInfo(struct psx_info *info);

/*
 * get_cop0_status() gets Coprocessor 0 status register
 */
 
unsigned int get_cop0_status();

/*
 * set_cop0_status() sets Coprocessor 0 status register
 */

unsigned int set_cop0_status(unsigned int sreg);

/*
 * get_cop0_epc() gets the contents of the program counter when the 
 * last exception happened.
 */
 
unsigned int get_cop0_epc();

// Root counter functions

int SetRCnt(int spec, unsigned short target, unsigned int mode);
int GetRCnt(int spec);
int StartRCnt(int spec);
int StopRCnt(int spec);

#endif

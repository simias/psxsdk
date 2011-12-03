#ifndef _PSXPAD_H
#define _PSXPAD_H

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

void QueryPAD(int pad_n, unsigned char *in, unsigned char *out, int len);
void pad_read_raw(int pad_n, unsigned char *arr);
void pad_escape_mode(int pad_n, int enable);
void pad_enable_vibration(int pad_n);
void pad_set_vibration(int pad_n, unsigned char small, unsigned char big);

#endif

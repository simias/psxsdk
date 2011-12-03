#ifndef _PSXSDK_MEMCARD_H
#define _PSXSDK_MEMCARD_H

typedef struct
{
	unsigned char num_of_icons;
	unsigned char num_of_slots;
	char name[64]; // Slot Name is in Shift-JIS encoding
	                           // (displayed by memory card manager)
	unsigned short clut[16];
	unsigned char icon[3][128]; // 16 x 16 x 4 bits icons
	unsigned int product_code; // Max 99999
	unsigned char devel_name[8]; // Development name (for internal purposes)
}McdSlotDef;

void McdDefToHeader(McdSlotDef *def, unsigned char *hdr);
int McdWriteData(McdSlotDef *def, int cardn, unsigned char *data);

// Memory card BIOS routines
int _card_info(int chan);
int _card_load(int chan);
int _card_read(int chan, int block, unsigned int *buf);
int _card_status(int drv);
int _card_write(int chan, int block, unsigned int *buf);

#endif

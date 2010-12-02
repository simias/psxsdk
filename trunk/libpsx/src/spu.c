/*
 * PSXSDK
 *
 * Sound Processing Unit Functions
 * Based on code from James Higgs's PSX lib and on code by bitmaster
 */

#include <stdio.h>
#include <psx.h>

#define SPU_ADDR				*((unsigned short*)0x1f801da6)
#define SPU_DATA				*((unsigned short*)0x1f801da8)
#define SPU_CONTROL 			*((unsigned short*)0x1f801daa)
#define SPU_STATUS			*((unsigned short*)0x1f801dac)
#define SPU_STATUS2			*((unsigned short*)0x1f801dae)
#define SPU_MVOL_L			*((unsigned short*)0x1f801d80)
#define SPU_MVOL_R			*((unsigned short*)0x1f801d82)
#define SPU_REVERB_L			*((unsigned short*)0x1f801d84)
#define SPU_REVERB_R			*((unsigned short*)0x1f801d86)
#define SPU_KEY_ON1           		*((unsigned short*)0x1f801d88)
#define SPU_KEY_ON2	            	*((unsigned short*)0x1f801d8a)
#define SPU_KEY_OFF1           		*((unsigned short*)0x1f801d8c)
#define SPU_KEY_OFF2            	*((unsigned short*)0x1f801d8e)
#define SPU_KEY_FM_MODE1		*((unsigned short*)0x1f801d90)
#define SPU_KEY_FM_MODE2		*((unsigned short*)0x1f801d92)
#define SPU_KEY_NOISE_MODE1		*((unsigned short*)0x1f801d94)
#define SPU_KEY_NOISE_MODE2		*((unsigned short*)0x1f801d96)
#define SPU_KEY_REVERB_MODE1		*((unsigned short*)0x1f801d98)
#define SPU_KEY_REVERB_MODE2		*((unsigned short*)0x1f801d9a)
#define SPU_CD_MVOL_L			*((unsigned short*)0x1f801db0)
#define SPU_CD_MVOL_R			*((unsigned short*)0x1f801db2)
#define SPU_EXT_VOL_L			*((unsigned short*)0x1f801db4)
#define SPU_EXT_VOL_R			*((unsigned short*)0x1f801db6)
#define SPU_REVERB_WORK_ADDR		*((unsigned short*)0x1f801da2)
#define SPU_VOICE_BASE_ADDR(x)		(0x1f801c00 + (x<<4))

// DPCR and other DMA defines will be eventually shared between GPU and SPU

#define DPCR				*((unsigned int*)0x1f8010f0)

void SsVoiceVol(int voice, unsigned short left, unsigned short right)
{
	unsigned short *a = (unsigned short*)SPU_VOICE_BASE_ADDR(voice);

	a[0] = left;
	a[1] = right;
}

void SsVoicePitch(int voice, unsigned short pitch)
{
	unsigned short *a = (unsigned short*)SPU_VOICE_BASE_ADDR(voice);
	
	a[2] = pitch;
}

void SsVoiceStartAddr(int voice, unsigned int addr)
{
// address given is real address, then it is divided by eight when written to the register	
// example: SSVoiceStartAddr(0, 0x1008) , writes 0x201 on the register which means 0x1008
	
	unsigned short *a = (unsigned short*)SPU_VOICE_BASE_ADDR(voice);
	
	a[3] = (addr >> 3);
}

void SsVoiceADSRRaw(int voice, unsigned short level, unsigned short rate)
{
	unsigned short *a = (unsigned short*)SPU_VOICE_BASE_ADDR(voice);

	a[4] = level;
	a[5] = rate;
}

void SsVoiceRepeatAddr(int voice, unsigned int addr)
{
// only valid after KeyOn
// the explanation for SSVoiceStartAddr() is valid for this function as well

	unsigned short *a = (unsigned short*)SPU_VOICE_BASE_ADDR(voice);
	
	a[7] = (addr >> 3);
}

void SsKeyOn(int voice)
{
	unsigned int i = 1 << voice;
	
	SPU_KEY_ON1 = i & 0xffff;
	SPU_KEY_ON2 = i >> 16;
	
/*	while(SPU_KEY_ON1 != (i & 0xffff));
	while(SPU_KEY_ON2 != (i >> 16));
*/
}
	
void SsKeyOff(int voice)
{
	unsigned int i = 1 << voice;
	
	SPU_KEY_OFF1 = i & 0xffff;
	SPU_KEY_OFF2 = i >> 16;
}

void SsKeyOnMask(int mask)
{
	SPU_KEY_ON1 = mask & 0xffff;
	SPU_KEY_ON2 = mask >> 16;
}

void SsKeyOffMask(int mask)
{
	SPU_KEY_OFF1 = mask & 0xffff;
	SPU_KEY_OFF2 = mask >> 16;
}
	
void SsWait()
{
	while(SPU_STATUS2 & 0x7ff);
}

void SsInit()
{
	int x;

	printf("Initializing SPU (Sound Synthesizer)...\n");
	
	DPCR |= 0xB0000;
	
	SPU_MVOL_L = 0x3fff;
	SPU_MVOL_R = 0x3fff;	

	SPU_CONTROL = 0x0;
	SsWait();

	SPU_STATUS = 0x4; // Must be done, but not totally understood

	while(SPU_STATUS2 & 0x7ff);
	
	SPU_REVERB_L = 0x0;
	SPU_REVERB_R = 0x0;

	// All keys off

	SPU_KEY_OFF1 = 0xFFFF;
	SPU_KEY_OFF2 = 0xFFFF;

	// Switch FM, reverb and noise off
	SPU_KEY_FM_MODE1 = 0x0;
	SPU_KEY_FM_MODE2 = 0x0;
	SPU_KEY_NOISE_MODE1 = 0x0;
	SPU_KEY_NOISE_MODE2 = 0x0;
	SPU_KEY_REVERB_MODE1 = 0x0;
	SPU_KEY_REVERB_MODE2 = 0x0;

	// set CD master volume to 0 (mute it)
	SPU_CD_MVOL_L = 0x0;
	SPU_CD_MVOL_R = 0x0;

	// set external input volume to 0 (mute it)
	SPU_EXT_VOL_L = 0x0;
	SPU_EXT_VOL_R = 0x0;

	// set volume of all voices to 0 and adsr to 0,0
	
	for(x = 0; x < 24; x++)
	{
		SsVoiceVol(x, 0, 0);		
		SsVoiceADSRRaw(x, 0, 0);
	}

	SsWait();

	SPU_CONTROL = 0xC000; // SPU is on
	SPU_REVERB_WORK_ADDR = 0xFFFE; // Reverb work address in SPU memory, 0x1fff * 8 = 0xFFF8

	printf("SPU/SS Initialized.\n");
}

// SsUpload is originally based on code by bitmaster
void SsUpload(void *addr, int size, int spu_addr)
{
	short spu_status; 
	int block_size;
	short *ptr;
	short d;
	int i;

	spu_status = SPU_STATUS2 & 0x7ff;

	SPU_ADDR = spu_addr >> 3;

	for(i=0;i<100;i++); // Waste time...

	ptr = (short *) addr;

	while(size > 0) 
	{
		block_size = ( size > 64 ) ? 64 : size; 

		for( i = 0; i < block_size; i += 2 )
			SPU_DATA = *ptr++;     

		d = SPU_CONTROL;
		d = ( d & 0xffcf ) | 0x10;
		SPU_CONTROL = d;	// write Block to SPU-Memory

		for(i=0;i<100;i++) // Waste time

		while(SPU_STATUS2 & 0x400);

		for(i=0;i<200;i++); // Waste time

		size -= block_size;
	}     

	SPU_CONTROL &= 0xffcf;

	while( ( SPU_STATUS2 & 0x7ff ) != spu_status ); 
}

unsigned short SsFreqToPitch(int hz)
{
// Converts a normal samples per second frequency value in Hz
// in a pitch value

// i.e. 44100 -> 0x1000, 22050 -> 0x800	
	
	return (hz << 12) / 44100;
}

int SsReadVag(SsVag *vag, void *data)
{
	unsigned int *i = data;
	
	if(strncmp(data, "VAGp", 4) != 0)
		return 0;
	
	vag->version = i[1];
	vag->data_size = i[3];
	vag->sample_rate = i[4];
	memcpy(vag->name, &i[8], 16);
	vag->data = &i[12];
	
	return 1;
}

void SsUploadVag(SsVag *vag, int spu_addr)
{
	SsUpload(vag->data, vag->data_size, spu_addr);
}

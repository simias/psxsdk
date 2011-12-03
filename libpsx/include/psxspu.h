#ifndef _SPU_H
#define _SPU_H

#define SPU_DATA_BASE_ADDR		0x1010

typedef struct
{
	unsigned int version;
	unsigned int data_size;
	unsigned int sample_rate;
	unsigned char name[16];
	void *data;
	unsigned int spu_addr;
	char cur_voice;
}SsVag;

void SsVoiceVol(int voice, unsigned short left, unsigned short right);
void SsVoicePitch(int voice, unsigned short pitch);
void SsVoiceStartAddr(int voice, unsigned int addr);
void SsVoiceADSRRaw(int voice, unsigned short level, unsigned short rate);
void SsVoiceRepeatAddr(int voice, unsigned int addr);
void SsKeyOn(int voice);
void SsKeyOff(int voice);
void SsWait();
void SsInit();
void SsUpload(void *addr, int size, int spu_addr);
unsigned short SsFreqToPitch(int hz);

// The functions below are for a simpler but less powerful interface which
// uses a VAG structure, initially loaded from a file

int SsReadVag(SsVag *vag, void *data);
void SsUploadVagEx(SsVag *vag, int spu_addr);
void SsUploadVag(SsVag *vag);
void SsPlayVag(SsVag *vag, unsigned char voice, unsigned short vl, 
	unsigned short vr);
void SsStopVag(SsVag *vag);
void SsResetVagAddr();


#endif

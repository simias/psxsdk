// MODplay for the PS1
// Music Module Player
// Supports ProTracker (.mod) and Composer 669 (.669) music module formats

// Requires libADPCM!
// 669 timing is not very good and a bit wrong...

// If NO_PSX_LIB is defined, no parts using PSXSDK functions are compiled
// This is useful if you want to use the library to handle module files in tools

#ifndef NO_PSX_LIB
#include <psx.h>
#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#ifndef NO_PSX_LIB
#include <adpcm.h>
#endif

#include "modplay.h"

// Configuration defines

// Size of ADPCM buffer used by MODUploadSamples when
// converting 8-bit unsigned PCM samples to PS1 ADPCM format
// By default it is set to 0x4000, 16 kilobytes

#define	ADPCM_BUFFER_SIZE		0x4000

// Almost all data from the MOD is copied into another location
// in memory for the ModMusic structure when you use MODLoad()
// This means your free memory should be roughly the double of 
// the size of the MOD you're loading.

// WRITE MODUNLOAD!!

int modplay_base_voice = 0;
int modplay_max_vol = 0x3fff;
int modplay_chan_vols[8];
int modplay_int_cnt = 0;
int modplay_samples_off[32];
int modplay_chan_mask = 0;
unsigned char modplay_adpcm_buffer[ADPCM_BUFFER_SIZE];

ModMusic *MODLoad_MOD(void *d);
void MODPlay_MOD(ModMusic *m, int *t);

ModMusic *MODLoad_669(void *d);
void MODPlay_669(ModMusic *m, int *t);

ModMusic *MODLoad(void *d)
{
	// Check if the module file we're loading is in Composer 669 format
	unsigned char *c = (unsigned char*)d;
	
	if(c[0] == 'i' && c[1] == 'f')
	{
		printf("Loading 669...\n");
		return MODLoad_669(d);
	}
	
	// If the module file was in no other format, assume the module file is
	// in ProTracker format. There's no real way to detect a ProTracker module
	// file 100% correctly so this will do.
	
	return MODLoad_MOD(d);
}

void MODUnload(ModMusic *m)
{
	int x;
	
	free(m->pattern_data);
	
	for(x = 0; x < m->sample_num; x++)
	{
		if(m->sample[x].length)
			free(m->sample[x].data);
	}
	
	free(m);
}

#ifdef NO_PSX_LIB
void MODPlay_func(ModMusic *m, int c, int s, int p, int vl, int vr)
{
	// Just a stub
}
#else
void MODPlay_func(ModMusic *m, int c, int s, int p, int vl, int vr)
{
	int v = c + modplay_base_voice;
//	static int mask = 0;
	
	if(s != -1)
	{
	//	SsKeyOff(v);
		SsVoicePitch(v,  p);
	}
	
	if(modplay_max_vol != 0x3fff)
	{
		vl = (modplay_max_vol * vl) / 0x4000;
		vr = (modplay_max_vol * vr) / 0x4000;
		vl&=0x3fff;
		vr&=0x3fff;
	}
	
	SsVoiceVol(v, vl, vr);
	
	if(s != -1)
	{
		SsVoiceStartAddr(v, modplay_samples_off[s]);
		modplay_chan_mask|=(1<<v);
		
		//if(c == (m->channel_num - 1))
		//{
			//SsKeyOnMask(mask);
			//mask = 0;
		//}
		//SsKeyOn(v);
	}
}
#endif

void MODPlay(ModMusic *m, int *t)
{
	modplay_chan_mask = 0;
	
	switch(m->fmt)
	{
		case MOD_FMT_MOD:
			MODPlay_MOD(m, t);
		break;
		case MOD_FMT_669:
			MODPlay_669(m,t);
		break;
	}
	
	//printf("modplay_chan_mask = %d\n", modplay_chan_mask);
#ifndef NO_PSX_LIB
	SsKeyOnMask(modplay_chan_mask);
#endif
}

void MODStop(ModMusic *m)
{
#ifndef NO_PSX_LIB
	int mask = 0;
	int x;
	
	for(x = 0; x<m->channel_num; x++)
		mask|=1<<(modplay_base_voice+x);
	
	SsKeyOffMask(mask);
#endif
}

#ifndef NO_PSX_LIB
int MODUploadSamples(ModMusic *m, int base_addr)
{
	int x, b;
	
	if(base_addr == -1)
		base_addr = SPU_DATA_BASE_ADDR;

	modplay_samples_off[0] = base_addr;
	
	for(x = 0; x < m->sample_num; x++)
	{
		b = SsAdpcmPack(m->sample[x].data, modplay_adpcm_buffer,
			m->sample[x].length, FMT_U8, sizeof(modplay_adpcm_buffer), 0);
		SsUpload(modplay_adpcm_buffer, b, modplay_samples_off[x]);
		
		if(x!=30)
			modplay_samples_off[x+1] = modplay_samples_off[x]+b;		
	}

	return modplay_samples_off[x];
}

int MOD4PSX_Upload(void *d, int base_addr)
{
	unsigned char *c = d;
	int x;
	int o;
	int ro;
	int sz;
	int n;
	
// Check magic string
	
	if(strncmp(c, "_mod4psx", 8) != 0)
		return -1;
	
	o = 12;
	n = *((int*)(c+8));
	
	if(base_addr == -1)
		modplay_samples_off[0] = SPU_DATA_BASE_ADDR;
	else
		modplay_samples_off[0] = base_addr;
	
	printf("Number of samples: %d\n", n);
	
	for(x = 0; x < n; x++)
	{
// Get size
		sz = *((int*)(c+o));
		printf("Size: %d\n", sz);
// Ignore eight reserved bytes (for future expension)
		o+=12;
		SsUpload(c+o, sz, modplay_samples_off[x]);
		
		if(x!=30)
			modplay_samples_off[x+1] = modplay_samples_off[x]+sz;
		
		o += sz;
	}
	
	return modplay_samples_off[x];
}
	
#endif

void MODSetBaseVoice(int base_voice)
{
	modplay_base_voice = base_voice;
}

void MODSetMaxVolume(unsigned short max_volume)
{
	// Default is 0x3fff.
	// Valid values 0 (MUTE) - 0x3FFF (MAX)
	
	modplay_max_vol = max_volume & 0x3fff;
}

#ifdef NO_PSX_LIB

// Some code might use this, so use a stub.

int SsFreqToPitch(int hz)
{
	return 0;
}

#endif

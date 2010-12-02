#ifndef _MODPLAY_H
#define _MODPLAY_H

enum
{
	MOD_FMT_MOD, // Ultimate SoundTracker / NoiseTracker / ProTracker
	MOD_FMT_669, // Composer 669
};

typedef struct
{
	unsigned char name[22];
	unsigned short length; // Length in words
	char finetune;
	unsigned char volume;
	unsigned short repeat_off;
	unsigned short repeat_len;
	unsigned char *data;
}ModSample;

typedef struct
{
	unsigned char title[20];
	int sample_num;
	int channel_num;
	ModSample sample[32];
	unsigned char song_pos_num;
	unsigned char pattern_tbl[128];
	unsigned char id[4];
	unsigned char pattern_num; // Number of patterns
	unsigned char *pattern_data;
	
	unsigned char tempo_list[128]; // For 669
	unsigned char break_loc_list[128]; // For 669
	
// Runtime data
	unsigned char song_pos;
	unsigned char pat_pos; // 0-63
	int divisions_sec;
	unsigned char beats_minute;
	unsigned char ticks_division;
	unsigned short old_periods[8];
	unsigned char old_samples[8];
	unsigned char arpeggio_timer[8];
	unsigned char arpeggio_sam[8];
	unsigned char arpeggio_x[8];
	unsigned char arpeggio_y[8];
	
	unsigned char cur_tempo; // For 699
	unsigned short cur_ticks; // For 669, timing
	
	int fmt;
}ModMusic;

// Allocate a ModMusic structure and copy data to it from 
// data in memory containing a ProTracker module file

ModMusic *MODLoad(void *d);

// Play a tick of a music 
// This has to be called 60 / 50 times per second
// t is a pointer to an int which contains how many times
// the mod file has to be played
// t = 1, play once
// t = 2, play twice
// ...
// t = -1, loop endlessly
//
// MODPlay decreases the value referenced by t every time
// the music finishes. Then if t != 0, the music is restarted
// from the beginning. If t == 0, MODPlay does nothing.
// Set the variable pointed by t when you want to set the number of times again!

void MODPlay(ModMusic *m,int *t);

// Upload the samples of the module music to Sound RAM
// base_addr is the Sound RAM address to start from when uploading to Sound RAM
// Specifying base_addr as -1 sets the base address to the start of the section for sound data in Sound RAM
// (SPU_DATA_BASE_ADDR). base_addr must be a multiply of 8.
// This function returns the sound address after all the uploaded samples

int MODUploadSamples(ModMusic *m, int base_addr);

// Sets the SPU voice to use as the first channel when playing music.
// The voice for the second channel will then be this (value+1), and so on...
// Usually the base voice is 0. A MOD file can have up to eight channels, so take care of that.
// A 669 file can always have 8 channels.

void MODSetBaseVoice(int base_voice);

// Internal function...

void MODPlay_func(ModMusic *m, int c, int s, int p, int vl, int vr);

extern int modplay_int_cnt;

#endif

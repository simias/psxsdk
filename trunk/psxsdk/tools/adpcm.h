#ifndef _PSX_ADPCM_H
#define _PSX_ADPCM_H

enum
{
	FMT_U8, // unsigned 8-bit
	FMT_S16, // signed 16-bit
};

int SsAdpcmPack(void *pcm_data, void *adpcm_data, int sample_len,
				int sample_fmt, int adpcm_len, int enable_looping,
				int loop_start);

#endif

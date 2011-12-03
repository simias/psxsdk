// Composer 669 format support for MODPlay

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "modplay.h"
#include "c669tbl.h" // PSX pitch table for Composer 669 modules

ModMusic *MODLoad_669(void *d)
{
	unsigned char *c = d;
	int mp=0x6e;
	int x;
	ModMusic *m;
	
	m = (ModMusic*)malloc(sizeof(ModMusic));

// 669 has eight channels
	m->channel_num = 8;
	
// Get number of samples	
	m->sample_num = c[mp++];
// Get number of patterns
	m->pattern_num = c[mp++];
// Loop order number ???
// Ignore this for now.
	//printf("Loop ordaru ! %d\n", c[mp]);
	mp++;
// Get order list (pattern table)
	memcpy(m->pattern_tbl,&c[mp],128);
	mp+=128;
// Get tempo list for patterns
	memcpy(m->tempo_list,&c[mp],128);
	mp+=128;
// Get break location for patterns
	memcpy(m->break_loc_list, &c[mp], 128);
	mp+=128;

	for(x = 0; x < m->sample_num; x++)
	{
		memcpy(m->sample[x].name, &c[mp], 13);
		mp+=13;
		m->sample[x].length = c[mp++];
		m->sample[x].length |= (c[mp++]<< 8);
		m->sample[x].length |= (c[mp++]<<16);
		m->sample[x].length |= (c[mp++]<<24);
		
		// Ignore looping stuff for now !
		mp+=8;
	}
	
	m->pattern_data = malloc(m->pattern_num * 0x600);
	memcpy(m->pattern_data, &c[mp], m->pattern_num * 0x600);
	mp+=m->pattern_num * 0x600;
	
	for(x = 0; x < m->sample_num; x++)
	{
		/*memcpy(m->sample[x].name, &c[mp], 13);
		mp+=13;
		m->length = c[mp++];
		m->length |= (c[mp++]<< 8);
		m->length |= (c[mp++]<<16);
		m->length |= (c[mp++]<<24);
		
		// Ignore looping stuff for now !
		mp+=8;*/
		m->sample[x].data = malloc(m->sample[x].length);
		memcpy(m->sample[x].data, &c[mp], m->sample[x].length);
		mp += m->sample[x].length;
	}
	
	m->song_pos = 0;
	m->pat_pos = 0;
	m->fmt = MOD_FMT_669;
	m->cur_tempo = m->tempo_list[m->pattern_tbl[m->song_pos]];
	
	//for(x = 0; x < 128; x++)
	//	printf("order list = %d\n", m->pattern_tbl[x]);
	
	x = 0;
//	while(m->pattern_tbl[x] != 255 && x < 128)
//	{
	while(x < 128)	
	{
		if(m->pattern_tbl[x] == 255)
			break;
		
		x++;
	}
	
	//for(x = 0; x < 128; x++)
	//	printf("break_loc_list[%d] = %x\n", x, m->break_loc_list[x]);
	
	m->song_pos_num = x;
	m->cur_ticks = m->cur_tempo;
	m->cur_ticks *= 78 - m->cur_tempo;
	m->cur_ticks = (60 / (m->cur_ticks / 60));
	
	return m;
}

void MODPlay_669(ModMusic *m, int *t)
{
	int x = 0;
	int cur_pat = m->pattern_tbl[m->song_pos];
	int cur_pat_pos = m->pat_pos;
	unsigned char b[3];
	int p,s,v,e,f,ti;
	
	if(*t == 0)
		return;
	
	if(modplay_int_cnt != m->cur_ticks)
	{
		modplay_int_cnt++;
		return;
	}
	
	for(x = 0; x < 8; x++)
	{
		memcpy(b, &m->pattern_data[(cur_pat * (24*64))+(cur_pat_pos * 24) + (x*3)], 3);

		if(b[0] == 0xff) // No note
		{
			modplay_int_cnt = 0;
			continue;
		}
		
		// 4 bit volume
		
		v = b[1] & 0xf;
		v<<=9;
		
		if (b[0] == 0xfe) // Only change volume
		{
			if(x & 1)
				MODPlay_func(m, x, -1, 0,  0, v);
			else
				MODPlay_func(m, x, -1, 0, v, 0);
		
			modplay_int_cnt = 0;
			continue;
		}
		
		// Get pitch (note value)
		
		p = (b[0] >> 2) & 0x3f;
		
		// 6 bit instrument number
		
		s = (b[0] & 3) << 4;
		s |= (b[1] >> 4) & 0xf;
		

		// Command
		
		e = b[2];
				
		// channels = LRLRLRLR
		
		if(x & 1)
			MODPlay_func(m, x, s, modplay_c669_pitch_tbl[p-12],  0, v);
		else
			MODPlay_func(m, x, s, modplay_c669_pitch_tbl[p-12], v, 0);
		
		modplay_int_cnt=0;
	}
	
	m->pat_pos++;
	
	if(m->pat_pos > 63 || m->pat_pos > m->break_loc_list[cur_pat])
	{
		m->pat_pos = 0;
		m->song_pos++;
		m->cur_tempo = m->tempo_list[m->pattern_tbl[m->song_pos]];
		m->cur_ticks = m->cur_tempo;
		m->cur_ticks *= 78 - m->cur_tempo;
		m->cur_ticks = (60 / (m->cur_ticks / 60));
	}
	
	if(m->song_pos == m->song_pos_num)
	{
		m->song_pos = 0;
		*t-=1;
		m->cur_tempo = m->tempo_list[m->pattern_tbl[m->song_pos]];
		m->cur_ticks = m->cur_tempo;
		m->cur_ticks *= 78 - m->cur_tempo;
		m->cur_ticks = (60 / (m->cur_ticks / 60));
		MODStop(m);
	}
	
}

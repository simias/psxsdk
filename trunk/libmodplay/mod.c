// Ultimate SoundTracker / NoiseTracker / ProTracker module file support for MODPlay

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "modplay.h"
#include "modtbl.h" // Period -> frequency table for ProTracker MODs


ModMusic *MODLoad_MOD(void *d)
{
	unsigned char *c = d;
	ModMusic *m;
	int x;
	int mp=0;
	int y;
	
// Allocate memory for mod structure	
	m = (ModMusic*)malloc(sizeof(ModMusic));

// Get title	
	memcpy(m->title, &c[0], 20);

// For now let's assume there are 32 samples...	
	mp+=20;	

	memcpy(m->id, &c[0x438], 4);
	
// If there is not a valid ID, this is the start of pattern data,
// otherwise jump four bytes	
	
	if(strncmp(m->id, "M.K.",4) == 0 || strncmp(m->id, "FLT4", 4) == 0 ||
		strncmp(m->id, "M!K!",4) == 0 || strncmp(m->id, "4CHN", 4) == 0 ||
			strncmp(m->id, "6CHN", 4) == 0 || strncmp(m->id, "8CHN", 4) == 0)
		m->sample_num = 31;
	else
		m->sample_num = 15;

// Standard channel number is 4	
	
	if(strncmp(m->id, "6CHN", 4) == 0)
		m->channel_num = 6;
	else if(strncmp(m->id, "8CHN", 4) == 0)
		m->channel_num = 8;
	else
		m->channel_num = 4;
	
// Get sample information
	
	for(x = 0; x < m->sample_num; x++)
	{
	// Get sample name
		memcpy(m->sample[x].name, &c[mp], 22);
		mp+=22;
	// Get sample length	
		m->sample[x].length = (c[mp] << 8) | c[mp+1];
		m->sample[x].length*= 2;
		mp+=2;
	// Get finetune value
		m->sample[x].finetune = c[mp] & 0xf;
		
		if(m->sample[x].finetune & 0x8)
			m->sample[x].finetune|=0xf0;
	
		mp++;
	// Get sample volume
		m->sample[x].volume = c[mp];
		mp++;
	
	// Get sample repeat offset 
		m->sample[x].repeat_off = (c[mp] << 8) | c[mp+1];
		m->sample[x].repeat_off *= 2;
		mp+=2;
		
	// Get sample repeat length
		m->sample[x].repeat_len = (c[mp] << 8) | c[mp+1];
		m->sample[x].repeat_len *= 2;
		mp+=2;
	}
	
// Get number of song positons
	m->song_pos_num = c[mp++];
	
// Ignore this value...	
	mp++;

// Get pattern table
	memcpy(m->pattern_tbl, &c[mp], 128);
	mp+=128;
	
// Get ID (it is not assured that this value will be valid)
	memcpy(m->id, &c[0x438], 4);
// If there is not a valid ID, this is the start of pattern data,
// otherwise jump four bytes	
	
	if(strncmp(m->id, "M.K.",4) == 0 || strncmp(m->id, "FLT4", 4) == 0 ||
		strncmp(m->id, "M!K!",4) == 0 || strncmp(m->id, "4CHN", 4) == 0 ||
			strncmp(m->id, "6CHN", 4) == 0 || strncmp(m->id, "8CHN", 4) == 0)
		mp+=4;
	
// Get number of patterns
// This is actually done by scanning the pattern table for the highest value
	y = 0;
	
	for(x=0;x<128;x++)
	{
		//printf("%x, \n", m->pattern_tbl[x]);
		if(m->pattern_tbl[x] > y)
			y = m->pattern_tbl[x];
	}
	
	//printf("\n");
	
	m->pattern_num = y+1;
	
// Allocate memory for patterns...
	m->pattern_data = malloc(m->pattern_num * 1024);

// Get pattern data	
	memcpy(m->pattern_data,&c[mp],  m->pattern_num * 1024);
	mp += m->pattern_num * 1024;
	
// Allocate & Get sample data
	for(x = 0; x < m->sample_num; x++)
	{
		m->sample[x].data = malloc(m->sample[x].length);
		memcpy(m->sample[x].data,  &c[mp], m->sample[x].length);

// Convert to unsigned 8-bit format
// Most sound cards/programs nowadays want data in this format		
		for(y = 0; y < m->sample[x].length; y++)
			m->sample[x].data[y] ^= 0x80;
		
		mp += m->sample[x].length;
	}
	
	m->song_pos = 0;
	m->pat_pos = 0;
	m->divisions_sec = 7;
	m->beats_minute = 125;
	m->ticks_division = 6;
	
	for(x = 0; x<8;x++)
	{
		m->old_samples[x] = 1;
		m->old_periods[x] = 0;
		m->arpeggio_timer[x] = 0;
		m->arpeggio_sam[x] = 0;
	}
	
	m->fmt = MOD_FMT_MOD;
	
	return m;
}

void MODPlay_MOD(ModMusic *m,int *t)
{
	int cur_pat = m->pattern_tbl[m->song_pos];
	int cur_pat_pos = m->pat_pos;
	unsigned char b[4];
	int s, p, e,x,y;
	int do_not_increase_pat = 0;
	int v1, v2,v3,f;
	
	if(m->fmt == MOD_FMT_669)
	{
		MODPlay_669(m, t);
		return;
	}
	
	if(*t == 0)
		return;
	
	if(modplay_int_cnt != (50 / m->divisions_sec))
	{
		modplay_int_cnt++;
		
		for(x = 0; x < 4; x++)
		{
			if(m->arpeggio_sam[x])
			{
				if(m->arpeggio_timer[x] == 2)
					v2 = m->arpeggio_x[x];
				else if(m->arpeggio_timer[x] == 1)
					v2 = m->arpeggio_y[x];
				else if(m->arpeggio_timer[x] == 0)
				{
					v2 = m->old_periods[x];
					//goto away;
				}
					
				m->arpeggio_timer[x]--;

				
				for(v1 = 0; v1 < sizeof(modplay_pitch_per_tbl) / 4; v1++)
				{
					//printf("%d == %d?\n", m->old_periods[x], modplay_freq_per_tbl[v1<<1]);
					if(m->old_periods[x] == modplay_pitch_per_tbl[v1<<1])
					{
						v3 = modplay_pitch_per_tbl[((v1+v2)<<1)+1];
						break;
					}
				}
				
				//f = SsFreqToPitch(7159090/(v3*2));
				f = v3;
				
				if(m->arpeggio_sam[x] != 0)
				MODPlay_func(m, x, m->arpeggio_sam[x]-1, f,
					0x3fff, 0x3fff); // Volume is wrong here !!!
				
				 if(m->arpeggio_timer[x] == 0)
					 m->arpeggio_sam[x]=0;
			}
		}
		
		return;
	}
	
	for(x = 0; x < m->channel_num; x++)
	{
		memcpy(b, &m->pattern_data[(cur_pat * 1024) + (cur_pat_pos * (4*m->channel_num)) + (x*4)], 4);
		
	// Get sample
		s = (b[2] & 0xf0)>>4;
		s |= b[0] & 0xf0;
		

		
	// Get period
		p = b[1];
		p|= (b[0] & 0xf)<<8;
		p&=~(2048|1024);
	
		if(s != 0 && p==0)
			p = m->old_periods[x];	
		
		if(s == 0 && p != 0)
			s=m->old_samples[x];
			
	// Get effect
		e = b[3];
		e|= (b[2] & 0xf)<<8;
		
		v1 = m->sample[s-1].volume;
		
		switch(e & 0xf00)
		{
			case 0xc00: // Set volume
				if(s!=0)
					v1 = e & 0xff;
			break;
		}
		
		//printf("%d) ",x);
		
		f = -1;
		
		for(y = 0; y < sizeof(modplay_pitch_per_tbl) / 4; y++)
		{
			if(modplay_pitch_per_tbl[y<<1] == p)
			{
				f = modplay_pitch_per_tbl[(y<<1)+1];
				break;
			}
		}
				
		if(f==-1 && p!=0)
		{
			printf("Couldn't find period %d in table. Calculating it.\n", p);
			f = SsFreqToPitch(7159090/(p*2));
		}	
		
		v1 <<= 8;
		
		if(v1 >= 0x4000)
			v1 = 0x3fff;
		
		if(s && p!=0)
		{
			if(x == 0 || x == 3 || x == 4 || x == 7)
				MODPlay_func(m, x, s-1, f, v1, 0);
			else
				MODPlay_func(m, x, s-1, f, 0, v1);
		}
			
		switch(e & 0xf00)
		{
			/*case 0x000: // arpeggio
				if(e!=0)
				{
					v1 = (e & 0xf0) >> 4;
					v2 = e & 0xf;
					//PlayMOD_func(
					 printf("arpeggio = %d, %d\n", v1, v2);
					
					for(v3 = 0; v3 < sizeof(modplay_freq_per_tbl) / 4; v3++)
					{
						if(modplay_freq_per_tbl[v3 << 1] == p)
						{
							PlayMOD_func(m, x+4, s, modplay_freq_per_tbl[(v3<<1)+(v1<<1)], e);
							PlayMOD_func(m, x+8, s, modplay_freq_per_tbl[(v3<<1)+(v2<<1)], e);
							break;
						}
					}

				}
			break;
			case 0x300:
			//	PlayMOD_func(m, x, s, p, e);
			break;*/
			case 0xb00: // Position Jump
				m->song_pos = e & 0xff;
				m->pat_pos = 0;
				// printf("Jump to song pos %d\n", m->song_pos);
			
				// this fixes some mods which jump over the mod itself
			
				if(m->song_pos >= m->song_pos_num)
					m->song_pos = 0;
			
				do_not_increase_pat = 1;
			break;
			case 0xd00: // Pattern break
				m->song_pos++;
				m->pat_pos = (((e&0xf0)>>4)*10)+(e&0xf);
				// printf("Pattern break, newpatpos=%d\n", m->pat_pos);
			
				// this fixes some mods which jump over the mod itself
			
				if(m->song_pos >= m->song_pos_num)
					m->song_pos = 0;
			
				do_not_increase_pat = 1;
			break;
			case 0xf00: // Tempo
				/*v1 = (e & 0xf0) >> 4;
				v2 = e & 0xf;
				v3 = (v1*16)+v2;*/
			
				if((e & 0xff) <= 32)
					m->ticks_division = e & 0xff;
				else
					m->beats_minute = e & 0xff;
			
				m->divisions_sec = 24 * m->beats_minute;
				m->divisions_sec /= m->ticks_division;
				m->divisions_sec /= 60;
			break;
			case 0x000:
				if(e != 0)
				{
					m->old_periods[x] = p;
					m->arpeggio_timer[x] = 2;
					m->arpeggio_sam[x] = s;
					m->arpeggio_x[x] = (e >> 4) & 0xf;
					m->arpeggio_y[x] = e & 0xf;
				}
			break;
			default:
				break;
			//	printf("unhandled effect ! %d\n", (e>>8)&0xf);
			//break;
		}
		
		//modplay_chan_vols[x]  = m->sample[s-1].volume;
		if(s) m->old_samples[x] = s;
		if(p) m->old_periods[x] = p;
		modplay_int_cnt = 0;
	}
	
	if(!do_not_increase_pat)m->pat_pos++;
	

	
	
	if(m->pat_pos == 64)
	{
		m->song_pos++;
		if(m->song_pos >= m->song_pos_num)
		{
			*t-=1;
			m->song_pos = 0;
			
			m->divisions_sec = 7;
			m->beats_minute = 125;
			m->ticks_division = 6;
	
			for(x = 0; x<8;x++)
			{
				m->arpeggio_timer[x] = 0;
				m->arpeggio_sam[x] = 0;
			}
			
			MODStop(m);
		}
			
		m->pat_pos = 0;
	}
}

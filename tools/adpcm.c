/*
 * PCM -> ADPCM routines
 *
 * based on work by Bitmaster and extended
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include "adpcm.h"

#define PCM_BUFFER_SIZE 128*28

short pcm_buffer[PCM_BUFFER_SIZE];

void SsAdpcm_find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor);
void SsAdpcm_pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor);

int SsAdpcmPack(void *pcm_data, void *adpcm_data, int sample_len,
				int sample_fmt, int adpcm_len, int enable_looping,
				int loop_start)
{
    short *ptr;
    unsigned char *pcm_data_c = pcm_data;
    short *pcm_data_s = pcm_data;
    unsigned char *adpcm_data_c = adpcm_data;
  //  short *adpcm_data_s = adpcm_data;
    double d_samples[28];
    short four_bit[28];
    int predict_nr;
    int shift_factor;
    int flags;
    int size;
    int i, j, k;    
    unsigned char d;
   // char s[4];
   // int chunk_data;
   // short e;
   // short sample_size;
    //unsigned char c;
    int ap = 0;
    int sp = 0;
    
/*printf("pcm_data = %x, adpcm_data = %x, len = %x, fmt = %x, alen = %x,"
	 "loop = %x\n", pcm_data, adpcm_data, sample_len, sample_fmt,
		adpcm_len, enable_looping);
			for(i=0;i<8;i++)
				printf("I[%d] = %x\n", i, pcm_data_c[i]);*/
	
    //if(enable_looping)
//		flags = 6;
//	else
		flags = 0;  

while( sample_len > 0 ) {
        size = ( sample_len >= PCM_BUFFER_SIZE ) ? PCM_BUFFER_SIZE : sample_len; 
	    
	if(sample_fmt == FMT_U8)
	{
		for(i = 0; i < size; i++)
		{
			//c = fgetc(fp);
			pcm_buffer[i] = *(pcm_data_c++);
			pcm_buffer[i] ^= 0x80;
			pcm_buffer[i] <<= 8;
		}
	}
	else if(sample_fmt == FMT_S16)
	{
		//fread( wave, sizeof( short ), size, fp );
		memcpy(pcm_buffer, pcm_data_s, size * sizeof(short));
		pcm_data_s += size;
	}
	else
	{
		printf("%s, line %d: Unknown source sample format!, id=%d\n",__FUNCTION__,__LINE__,sample_fmt);  
		return 0;
	}
		
        i = size / 28;
        if ( size % 28 ) {
            for ( j = size % 28; j < 28; j++ )
                pcm_buffer[28*i+j] = 0;
            i++;
        }
        
        for ( j = 0; j < i; j++ ) {                                     // pack 28 samples
	    if ( sample_len < 28 && enable_looping == 0)
                flags = 1;
	    
	    if(enable_looping)
	    {
		    if(((loop_start/28)*28) == sp)
			flags = 6;
		    else
			flags = 2;
	    }
		
		
            ptr = pcm_buffer + j * 28;
            SsAdpcm_find_predict( ptr, d_samples, &predict_nr, &shift_factor );
            SsAdpcm_pack( d_samples, four_bit, predict_nr, shift_factor );
            d = ( predict_nr << 4 ) | shift_factor;
          //  fputc( d, vag );
	    adpcm_data_c[ap++] = d;
	    if(ap>=adpcm_len) goto adpcm_too_big;
          //  fputc( flags, vag );
	    adpcm_data_c[ap++] = flags;
	    if(ap>=adpcm_len) goto adpcm_too_big;
            for ( k = 0; k < 28; k += 2 ) {
                d = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
           //     fputc( d, vag );
	    adpcm_data_c[ap++] = d;
	    if(ap>=adpcm_len) goto adpcm_too_big;
            }
            sample_len -= 28;
	    sp += 28;
        }
    }
    
   // fputc( ( predict_nr << 4 ) | shift_factor, vag );
    adpcm_data_c[ap++] = ( predict_nr << 4 ) | shift_factor;
    if(ap>=adpcm_len) goto adpcm_too_big;
    
    if(enable_looping)
	//    fputc(3, vag);
	adpcm_data_c[ap++] = 3;
    else
//	fputc( 7, vag );            // end flag
	adpcm_data_c[ap++] = 7;
    
    if(ap>=adpcm_len) goto adpcm_too_big;
    
    for ( i = 0; i < 14; i++ )
      //  fputc( 0, vag );
	adpcm_data_c[ap++] = 0;

    if(ap>=adpcm_len) goto adpcm_too_big;

    return ap;
 
adpcm_too_big:    
    printf("%s: Resulting ADPCM data would have been larger than the output array length! Exiting %s.\n", __FUNCTION__, __FUNCTION__);
    return 0;
}



                  


void SsAdpcm_find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor )
{
    int i, j;
    double buffer[28][5];
    double min = 1e10;
    double max[5];
    double ds;
    int min2;
    int shift_mask;
    static double _s_1 = 0.0;                            // s[t-1]
    static double _s_2 = 0.0;                            // s[t-2]
    double s_0, s_1, s_2;
    double f[5][2] = { { 0.0, 0.0 },
                            {  -60.0 / 64.0, 0.0 },
                            { -115.0 / 64.0, 52.0 / 64.0 },
                            {  -98.0 / 64.0, 55.0 / 64.0 },
                            { -122.0 / 64.0, 60.0 / 64.0 } };

    for ( i = 0; i < 5; i++ ) {
        max[i] = 0.0;
        s_1 = _s_1;
        s_2 = _s_2;
        for ( j = 0; j < 28; j ++ ) {
            s_0 = (double) samples[j];                      // s[t-0]
            if ( s_0 > 30719.0 )
                s_0 = 30719.0;
            if ( s_0 < - 30720.0 )
                s_0 = -30720.0;
            ds = s_0 + s_1 * f[i][0] + s_2 * f[i][1];
            buffer[j][i] = ds;
            if ( fabs( ds ) > max[i] )
                max[i] = fabs( ds );
//                printf( "%+5.2f\n", s2 );
                s_2 = s_1;                                  // new s[t-2]
                s_1 = s_0;                                  // new s[t-1]
        }
        
        if ( max[i] < min ) {
            min = max[i];
            *predict_nr = i;
        }
        if ( min <= 7 ) {
            *predict_nr = 0;
            break;
        }
        
    }

// store s[t-2] and s[t-1] in a static variable
// these than used in the next function call

    _s_1 = s_1;
    _s_2 = s_2;
    
    for ( i = 0; i < 28; i++ )
        d_samples[i] = buffer[i][*predict_nr];

//  if ( min > 32767.0 )
//      min = 32767.0;
        
    min2 = ( int ) min;
    shift_mask = 0x4000;
    *shift_factor = 0;
    
    while( *shift_factor < 12 ) {
        if ( shift_mask  & ( min2 + ( shift_mask >> 3 ) ) )
            break;
        (*shift_factor)++;
        shift_mask = shift_mask >> 1;
    }
      
}

void SsAdpcm_pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor )
{
    static double f[5][2] = { { 0.0, 0.0 },
                            {  -60.0 / 64.0, 0.0 },
                            { -115.0 / 64.0, 52.0 / 64.0 },
                            {  -98.0 / 64.0, 55.0 / 64.0 },
                            { -122.0 / 64.0, 60.0 / 64.0 } };
    double ds;
    int di;
    double s_0;
    static double s_1 = 0.0;
    static double s_2 = 0.0;
    int i;

    for ( i = 0; i < 28; i++ ) {
        s_0 = d_samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
        ds = s_0 * (double) ( 1 << shift_factor );

        di = ( (int) ds + 0x800 ) & 0xfffff000;

        if ( di > 32767 )
            di = 32767;
        if ( di < -32768 )
            di = -32768;
            
        four_bit[i] = (short) di;

        di = di >> shift_factor;
        s_2 = s_1;
        s_1 = (double) di - s_0;

    }
}

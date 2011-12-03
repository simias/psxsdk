// util.c
// PSXSDK utility functions

// This is not a core part of the PSXSDK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psxutil.h>

const char *psxsdk_btn_names[] =
							     { "L2", "R2", "L1", "R1", "Triangle", "Circle", "Cross",
								 "Square", "Select", "Res1", "Res2", "Start",
								 "Up", "Right", "Down", "Left"};
								 
void *PSX_GetButtonName(unsigned short button, unsigned char *out, unsigned int out_len)
{
	int x;
	
	if(out_len)out[0] = 0;
	
	for(x = 0; x < 16; x++)
	{
		if(button & (1<<x))
		{
			strncat(out, psxsdk_btn_names[x], out_len);
			out_len -= strlen(out);
			strncat(out, "&", out_len);
			out_len--;
		}
	}
		
	if(strlen(out))
	{
		if(out[strlen(out) - 1] == '&')
			out[strlen(out) - 1] = 0;
	}
	else
		strncpy(out, "None", out_len);
}

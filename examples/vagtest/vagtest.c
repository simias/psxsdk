/*
 * vagtest
 */
 
#include <psx.h>
#include <stdio.h>
#include "vag1.h"
#include "vag2.h"

unsigned int prim_list[0x1000];

SsVag my_vag[2];

int main(int argc, char *argv[])
{
	PSX_Init();
	GsInit();
	SsInit();
	
	GsClearMem();
	GsSetList(prim_list);
	GsSetVideoMode(320, 240, EXAMPLES_VMODE);
	
	GsSetDispEnvSimple(0, 0);
	GsSetDrawEnvSimple(0, 0, 320, 240);
	
	GsLoadFont(960, 0, 960, 256);
	
	GsPrintFont(0, 16, "Press X to hear 1st VAG");
	GsPrintFont(0, 24, "Press O to hear 2nd VAG");
	
	GsDrawList();
	
	while(1);
}

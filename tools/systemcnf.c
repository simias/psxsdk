/*
 * systemcnf
 *
 * Small program that outputs a system.cnf file to standard output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
 
int tcb = 4;
int event = 16;
unsigned int stack = 0x801FFFF0;
 
int main(int argc, char *argv[])
{
	int x;

	if(argc < 2)
	{
		printf("systemcnf\n");
		printf("usage: systemcnf exe_name [tcb] [event] [stack_addr]\n");
		printf("\n");
		printf("This programs outputs a system.cnf file to standard output\n");
		printf("Only the exe_name argument is necessary because if the others\n");
		printf("are missing, the default values are used.\n");
		printf("Specify stack_addr in hexadecimal, without prefixes.\n");
		return 0;
	}
	
	if(argc >= 3)
		tcb = atoi(argv[2]);
		
	if(argc >= 4)
		event = atoi(argv[3]);
		
	if(argc >= 5)
		sscanf(argv[4],"%x",&stack);
	
	for(x=0;x<strlen(argv[1]);x++)
		argv[1][x] = toupper((int)argv[1][x]);
		
	printf("BOOT = cdrom:%s;1\n", argv[1]);
	printf("TCB = %d\n", tcb);
	printf("EVENT = %d\n", event);
	printf("STACK = %X", stack);
	
	return 0;
}

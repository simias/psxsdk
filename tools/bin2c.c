/*
 * bin2c: converts a file to a C array of bytes
 */
 
#include <stdio.h>

int main(int argc, char *argv[])
{
	char *name = "data";
	int n = 0;
	int c;
	
	if(argc >= 2)
		name = argv[1];
		
	printf("unsigned char %s_array[] =\n{\n", name);
	
	while((c=getchar()) != EOF)
	{
		printf("%d, ", c);
		n++;
		
		if(!(n & 15))
			printf("\n");
	}
	
	printf("};\n");
	
	return 0;
}

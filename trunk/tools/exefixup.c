/* 
 * exefixup.c v0.02.1 Andrew Kieschnick <andrewk@mail.utexas.edu>
 *                    (v0.02.1): Giuseppe Gatta <tails92@gmail.com>
 *
 * v0.02.1 changes: removed warnings
 *
 * displays PS-X EXE header information
 * offers to fix incorrect t_size
 * offers to pad to 2048-byte boundary for cd-rom use
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

unsigned int char2int(unsigned char *foo)
{
  return foo[3]*16777216 + foo[2]*65536 + foo[1]*256 + foo[0];
}

void int2char(unsigned int foo, unsigned char *bar)
{
  bar[3]=foo>>24;
  bar[2]=foo>>16;
  bar[1]=foo>>8;
  bar[0]=foo;
}

void usage(void)
{
  printf("Usage: exefixup <filename>\n\n");
  printf("\t<filename>\ta PS-X EXE file\n\n");
  printf("\tdisplays EXE header\n");
  printf("\toffers to correct a wrong t_size\n");
  printf("\toffers to pad to 2048-byte boundary\n\n");
  exit(0);
}

int main(int argc, char *argv[])
{
  FILE *exe;
  FILE *out;
  unsigned char data[8];
  char filename[256];
  int i;
  unsigned int header_data[12];
  unsigned int size;
  unsigned int padsize;
  signed char yesno='Z';

  printf("exefixup v0.02.1 Andrew Kieschnick <andrewk@mail.utexas.edu>\n\n");

  if (argc!=2)
    usage();

  strncpy(filename,argv[1],256);

  exe=fopen(filename, "r");

  strcat(filename, "-fixed"); /* output filename is same as input filename, but with -fix appended */

  if (!exe)
    {
      printf("ERROR: Can't open %s\n",filename);
      exit(-1);
    }

  for(i=0;i<8;i++)
    fscanf(exe, "%c", &data[i]);
  data[8]=0;

  if (strncmp((char*)data, "PS-X EXE", 8))
    {
      printf("ERROR: Not a PS-X EXE file\n");
      exit(-1);
    }
  
  for(i=0;i<12;i++)
    {
      fscanf(exe, "%c", &data[0]);
      fscanf(exe, "%c", &data[1]);
      fscanf(exe, "%c", &data[2]);
      fscanf(exe, "%c", &data[3]);
      header_data[i]=char2int(data);
    }

  printf("id\tPS-X EXE\n");
  printf("text\t0x%.8x\n", header_data[0]);
  printf("data\t0x%.8x\n", header_data[1]);
  printf("pc0\t0x%.8x\n", header_data[2]);
  printf("gp0\t0x%.8x\n", header_data[3]);
  printf("t_addr\t0x%.8x\n", header_data[4]);
  printf("t_size\t0x%.8x\n", header_data[5]);
  printf("d_addr\t0x%.8x\n", header_data[6]);
  printf("d_size\t0x%.8x\n", header_data[7]);
  printf("b_addr\t0x%.8x\n", header_data[8]);
  printf("b_size\t0x%.8x\n", header_data[9]);
  printf("s_addr\t0x%.8x\n", header_data[10]);
  printf("s_size\t0x%.8x\n\n", header_data[11]);

  fseek(exe, 0, SEEK_END);
  
  size=ftell(exe)-2048;

  padsize=2048-(size%2048);

  if (padsize!=2048)
    {
      printf("WARNING: EXE size is not a multiple of 2048!\n");
      while ((yesno!='Y')&&(yesno!='N'))
	{
	  printf("Write a padded EXE (to %s) ? ",filename);
	  scanf("%c%*c", &yesno);
	  yesno=toupper(yesno);
	}
      if (yesno=='Y')
	{
	  out = fopen(filename, "w");
	  
	  header_data[5]=size+padsize;

	  fprintf(out, "PS-X EXE");
	  for(i=0;i<12;i++)
	    {
	      int2char(header_data[i], data);
	      fprintf(out, "%c%c%c%c", data[0], data[1], data[2], data[3]);
	    }
	  
	  fseek(exe, 56, SEEK_SET);

	  for(i=0;i<size+1992;i++)
	    {
	      fscanf(exe, "%c", &data[0]);
	      fprintf(out, "%c", data[0]);
	    }
	  for(i=0;i<padsize;i++)
	    fprintf(out, "%c", 0);
	  
	  size=header_data[5];
	  fclose(out);	  
	}
    }
  
  yesno='Z';
  
  if (size!=header_data[5])
    {
      printf("WARNING: EXE header t_size does not match filesize-2048\n");
      printf("EXE header:\t 0x%.8x bytes\n", header_data[5]);
      printf("filesize-2048:\t 0x%.8x bytes\n", size);
      while ((yesno!='Y')&&(yesno!='N'))
	{
	  printf("Write a corrected EXE (to %s) ? ",filename);
	  scanf("%c%*c", &yesno);
	  yesno=toupper(yesno);
	}
      if (yesno=='Y')
	{
	  out = fopen(filename, "w");
	  
	  fprintf(out, "PS-X EXE");
	  for(i=0;i<5;i++)
	    {
	      int2char(header_data[i], data);
	      fprintf(out, "%c%c%c%c", data[0], data[1], data[2], data[3]);
	    }
	  int2char(size, data);
	  fprintf(out, "%c%c%c%c", data[0], data[1], data[2], data[3]);
	  for(i=6;i<12;i++)
	    {
	      int2char(header_data[i], data);
	      fprintf(out, "%c%c%c%c", data[0], data[1], data[2], data[3]);
	    }
	  
	  fseek(exe, 56, SEEK_SET);

	  for(i=0;i<size+1992;i++)
	    {
	      fscanf(exe, "%c", &data[0]);
	      fprintf(out, "%c", data[0]);
	    }
	  fclose(out);
	}
    }
  fclose(exe);
  return 0;
}


  
  

  

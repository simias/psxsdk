/*
 * small libc functions for the PSXSDK
 *
 * In this file, C functions either not implemented by the BIOS
 * or very broken in the BIOS are implemented independently
 *
 * Some functions, such as printf, have both a BIOS implementation
 * and an implementation here. The implementation here is prefixed
 * with libc_ to not make confusion.
 */

#define __IN_LIBC

#include <psx.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char onesec_buf[2048];
int errno;

FILE file_structs[256];

unsigned char file_state[256];



enum
{
	FDEV_UNKNOWN,
	FDEV_CDROM,
	FDEV_MEMCARD
};

unsigned int fmode_to_desmode(const char *fmode)
{
	char rmode[16];
	int x, y;
	
	y = 0;

	for(x=0;x<15;x++)
	{
		if(fmode[x] == 0)
			break;
		else
		{
			if(fmode[x] != 'b' && fmode[x] != 'f')
				rmode[y++] = fmode[x];
		}
	}
	
	rmode[y] = 0;
	
	if(strcmp(rmode, "r") == 0)
	{
		printf("Open for reading.\n");
		return O_RDONLY;
	}
	else if(strcmp(rmode, "r+") == 0)
	{
		printf("Open for reading and writing.\n");
		return O_RDWR;
	}
	else if(strcmp(rmode, "w") == 0)
	{
		printf("Open for writing.\n");
		return O_WRONLY | O_CREAT | O_TRUNC;
	}
	else if(strcmp(rmode, "w+") == 0)
	{
		printf("Open for writing. Truncate to zero or create file.\n");
		return O_RDWR | O_CREAT | O_TRUNC;
	}
	else if(strcmp(rmode, "a") == 0)
	{
		printf("Append; open for writing. Create file if it doesn't exist.\n");
		return O_WRONLY | O_APPEND;
	}
	else if(strcmp(rmode, "a+") == 0)
	{
		printf("Append; open for reading and writing. Create file if it doesn't exist.\n");
		return O_RDWR | O_APPEND | O_CREAT;
	}
	else
	{
		return 0;
	}
}

FILE *fdopen(int fildes, const char *mode)
{
// Adjust for malloc
	int x;

// Find a free file structure	
	for(x = 0; x < 256; x++)
	{
		if(file_structs[x].used == 0)
		{
			file_structs[x].used = 1;
			break;
		}
	}

// If we found no free file structure, return NULL pointer
	
	if(x == 256)
		return NULL;
		

	file_structs[x].fildes = fildes;
	file_structs[x].pos = lseek(fildes, 0, SEEK_CUR);
	file_structs[x].mode = fmode_to_desmode(mode);

	return &file_structs[x];
}

FILE *fopen(char *path, const char *mode)
{
	int fd;
	FILE *f;
	
	fd = open(path, fmode_to_desmode(mode));
	
	if(fd == -1)
		return NULL;
		
	f = fdopen(fd, mode);
	
	if(f == NULL)
		return NULL;
	
	f->dev = FDEV_UNKNOWN;
	
	if(strncmp(path, "cdrom", 5) == 0)
		f->dev = FDEV_CDROM;
	else if(strncmp(path, "bu", 2) == 0)
		f->dev = FDEV_MEMCARD;
	
	f->size = get_real_file_size(path);
		
	return f;
}

int fclose(FILE *stream)
{
	stream->used = 0;
	close(stream->fildes);
}

/*
 * fread doesn't require reads to be carried in block unit
 * Notice that however seeks on the CD drive will be very slow - so avoid using non block units
 *
 * This is done to make programming and porting easier 
 */

int fread(void *ptr, int size, int nmemb, FILE *f)
{
	int rsize = size * nmemb;
	int csize = rsize;
	int max;
	int sect_num = 0;
	int r;
	int nsect = (f->pos + rsize) >> 11; 
	nsect -= f->pos >> 11;
	nsect++;
	
	//printf("f->dev = %d, f->pos = %d, rsize = %d\n", f->dev, f->pos, rsize);

	if(f->dev == FDEV_CDROM)
	{
		// First sector
		lseek(f->fildes, f->pos & (~0x7ff), SEEK_SET);
		read(f->fildes, onesec_buf, 2048);
		
		max = 2048 - (f->pos & 2047);
		
		//printf("ptr(FIRST) = %d, %x\n", ptr, ptr);
		printf("rsize = %d\n", rsize);
		
		memcpy(ptr, onesec_buf + (f->pos & 2047), (rsize > max) ? max : rsize);
		
		// Middle sector
		ptr += max;
		
		//printf("ptr(MIDDLEsex) = %d, %x\n", ptr, ptr);
		nsect--;
		csize -= max;
		
		if(nsect > 1)
		{
			//lseek(f->fildes, (f->pos & (~0x7ff)) + 2048, SEEK_SET);

#warning "Check correctness of this calculation."
		
			/*if(rsize & 2047)
				sect_num = (rsize|2047)+1;
			else
				sect_num = rsize;
				
			sect_num -= 4096;*/
			
			//printf("read_middle=%d, sect_num = %d\n", read(f->fildes, ptr, sect_num), sect_num);
			
			read(f->fildes, ptr, (nsect - 1) * 2048);
			
			ptr += (nsect - 1) * 2048;
			csize -= (nsect - 1) * 2048;
			nsect = 1;
		}
		
		//printf("ptr(LAST) = %d, %x\n", ptr, ptr);
		
		if(nsect == 1)
		{
			// Last sector
			read(f->fildes, onesec_buf, 2048);
		
			memcpy(ptr, onesec_buf, csize);
		}
	}	

	f->pos+= rsize;	
	return rsize;
}

int fgetc(FILE *f)
{
	unsigned char c;
		
	if(f->pos >= f->size)
		return EOF;
	
	fread(&c, sizeof(char), 1, f);
		
	return (int)c;
}
	
int ftell(FILE *f)
{
	return f->pos;
}

int fseek(FILE *f, int offset, int whence)
{
	switch(whence)
	{
		case SEEK_SET:
			f->pos = offset;
		break;
		case SEEK_CUR:
			f->pos+= offset;
		break;
		case SEEK_END:
			f->pos = f->size + offset;
		break;
		default:
			f->pos = whence + offset;
		break;
	}
	
	return 0;
}

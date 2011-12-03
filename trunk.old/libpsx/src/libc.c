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
#include <fcntl.h>
#include <errno.h>

#define SPRINTF_ALT_FLAG			(1<<0)
#define SPRINTF_ZERO_FLAG			(1<<1)
#define SPRINTF_NEGFIELD_FLAG			(1<<2)
#define SPRINTF_SPACE_FLAG			(1<<3)
#define SPRINTF_SIGN_FLAG			(1<<4)

// sprintf() macros to calculate the real padding and to write it
// these were made to not repeat the code in the function
// they can only be used in sprintf()

// sprintf macros START

#define calculate_real_padding() \
	y = 1; \
	\
	for(x=0;x<=9;x++) \
	{ \
		if(x == 0) \
			pad_quantity--; \
		else \
		{ \
			if(arg / y) \
				pad_quantity--; \
		} \
	\
		y *= 10; \
	} \
	\
	if(pad_quantity < 0) pad_quantity = 0;

/*#define calculate_real_padding_hex() \
	for (x = 0; x < 8; x++) \
	{ \
		if(x == 0) \
			pad_quantity--; \
		else \
		{ \
		if((arg >> (x * 4)) & 0xf) \
			pad_quantity--; \
		} \
	}*/
	
#define calculate_real_padding_hex() \
	last = 0; \
	for (x = 0; x < 8; x++) \
		if((arg >> (x * 4)) & 0xf) \
			last = x; \
	\
	pad_quantity = (pad_quantity - 1) - last; \
	if(pad_quantity < 0) pad_quantity = 0;

#define write_padding() \
	if(!(flags & SPRINTF_NEGFIELD_FLAG)) \
		for(x = 0; x < pad_quantity; x++) \
		{ \
			if(flags & SPRINTF_ZERO_FLAG) \
					put_in_string(string, ssz, '0', string_pos++); \
			else \
					put_in_string(string, ssz, ' ', string_pos++); \
		}

#define write_neg_padding() \
	if(flags & SPRINTF_NEGFIELD_FLAG) \
	{ \
		for(x = 0; x < pad_quantity; x++) \
			put_in_string(string, ssz, ' ', string_pos++);\
	}

// sprintf macros END

char onesec_buf[2048];
int errno;

FILE file_structs[256];

unsigned char file_state[256];

enum
{
	SPRINTF_SIZE_CHAR,
	SPRINTF_SIZE_SHORT,
	SPRINTF_SIZE_LONG,
	SPRINTF_SIZE_LONG_LONG,
};

enum
{
	FDEV_UNKNOWN,
	FDEV_CDROM,
	FDEV_MEMCARD
};

void *memcpy(void *dst, void *src, int len)
{
	void *dst2 = dst;
	
	while(len--)
		*(((unsigned char*)dst++)) = *(((unsigned char*)src++));
		
	return dst2;
}

unsigned int get_arg_in_size(int size, unsigned int *arg, unsigned int check_sign)
{
	int s = 0;

	switch(size)
	{
		case SPRINTF_SIZE_CHAR:
			*arg &= 0xff;
			
			if(check_sign)
			{
				if(*arg & (1<<7))
				{
					*arg |= 0xffffff00;
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;
		case SPRINTF_SIZE_SHORT:
			*arg &= 0xffff;
			
			if(check_sign)
			{
				if(*arg & (1<<15))
				{
					*arg |= 0xffff0000;
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;
		case SPRINTF_SIZE_LONG:
			*arg &= 0xffffffff;

			if(check_sign)
			{
				if(*arg & (1<<31))
				{
					*arg = ~(*arg - 1);
					s = 1;
				}
			}
		break;
	}
	
	return s;
}	

int put_in_string(char *string, unsigned int sz, char c, int pos)
{
	if(pos>=sz)
		return 0;
	else
		string[pos] = c;
		
	return 1;
}
 
int vsnprintf(char *string, unsigned int size, char *fmt, va_list ap)
{
	int string_pos,fmt_pos;
	int l;
	unsigned int arg;
	unsigned char *argcp;
	unsigned char *argcp_tmp;
	int directive_coming = 0;
	int alternate_form = 0;
	int flags = 0;
	int argsize = 2; // int
	int x, y;
	unsigned int a, b;
	int empty_digit;
	int ssz = size - 1;
	int zero_flag_imp = 0;
	int pad_quantity = 0;
	int last;
	
	l = strlen(fmt);
	
	string_pos = 0;
	
	for(fmt_pos=0;fmt_pos<l;fmt_pos++)
	{
		if(directive_coming)
		{
			switch(fmt[fmt_pos])
			{
				case '%':
					put_in_string(string, ssz, '%', string_pos++);
					directive_coming = 0;
				break;
				case ' ':
					flags |= SPRINTF_SPACE_FLAG;
				break;
				case '#': // Specify alternate form
					flags |= SPRINTF_ALT_FLAG;
				break;
				case '+': // Specify sign in signed conversions
					flags |= SPRINTF_SIGN_FLAG;
				break;
				case '0': // Padding with zeros...
					if(zero_flag_imp == 0)
					{
						flags |= SPRINTF_ZERO_FLAG;
						zero_flag_imp = 1;
						//printf("Zero padding enabled!\n"); 
					}
					else
					{
						pad_quantity *= 10;
						//printf("pad_quantity = %d\n", pad_quantity);
					}	
				break;
				case '1' ... '9': // '...' cases are a GNU extension,
				                  // but they simplify a lot 
				                  
				        pad_quantity *= 10;
				        pad_quantity += fmt[fmt_pos] - '0';
				        zero_flag_imp = 1;
				        
				        //printf("pad_quantity = %d\n", pad_quantity);
				break;
				case '-': // Negative field flag
					if(flags & SPRINTF_ZERO_FLAG)
						flags &= ~SPRINTF_ZERO_FLAG;
				
					flags |= SPRINTF_NEGFIELD_FLAG;
				break;
				case 'h': // Half argument size
					if(argsize) argsize--;
				break;
				case 'l': // Double argument size
					if(argsize < 2) argsize = 2;
					else if(argsize < 3) argsize++;
				break;
				case 'd': // signed decimal
				case 'i':
					empty_digit = 1;
				
					arg = va_arg(ap, unsigned int);
					
					if(get_arg_in_size(argsize, &arg, 1))
					{
						put_in_string(string, ssz, '-', string_pos++);
						pad_quantity--;
					}
					else
					{
						if(flags & SPRINTF_SIGN_FLAG)
						{
							put_in_string(string, ssz, '+', string_pos++);
							pad_quantity--;
						}
					}
				
					/* Calculate how much padding we have to write */
					
					/*y = 1;
					
					for(x=0;x<=9;x++)
					{
						if(x == 0)
							pad_quantity--;
						else
						{
							if(arg / y)
								pad_quantity--;
						}
						
						y *= 10;
					}
					if(pad_quantity < 0) pad_quantity = 0;*/

					calculate_real_padding();
					
					//printf("Actual pad quantity = %d\n", pad_quantity);
					
					

					/*if(!(flags & SPRINTF_NEGFIELD_FLAG))
					{
						for(x = 0; x < pad_quantity; x++)
						{
							if(flags & SPRINTF_ZERO_FLAG)
								put_in_string(string, ssz, '0', string_pos++);
							else
								put_in_string(string, ssz, ' ', string_pos++);
						}
					}*/
				
					write_padding();

					for(x=9;x>=0;x--)
					{
						a = 1;
						for(y = 0; y<x; y++)
							a *= 10;
							
						b = (arg/a);
						
						if(b>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{	
							arg -= b*a;
						
							put_in_string(string, ssz, b + '0', string_pos++);
						}
					}
					
					/*if(flags & SPRINTF_NEGFIELD_FLAG)
					{
						for(x = 0; x < pad_quantity; x++)
								put_in_string(string, ssz, ' ', string_pos++);
					}*/
					write_neg_padding();
					
					directive_coming = 0;
				break;
				case 'u': // unsigned decimal
					empty_digit = 1;
				
					arg = va_arg(ap, unsigned int);
				
					get_arg_in_size(argsize, &arg, 0);
				
					calculate_real_padding();
					write_padding();

					for(x=9;x>=0;x--)
					{
						a = 1;
						for(y = 0; y<x; y++)
							a *= 10;
						
						
							
						b = (arg/a);
						
						if(b>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{	
							arg -= b*a;
						
							put_in_string(string, ssz, b + '0', string_pos++);
						}
					}
					
					write_neg_padding();

					directive_coming = 0;
				break;		
				case 'x': // Hexadecimal
				case 'X': // Hexadecimal with big letters
				case 'p': // Hexadecimal with small letters with '0x' prefix
					empty_digit = 1;
					arg = va_arg(ap, unsigned int);
					
					get_arg_in_size(argsize, &arg, 0);
				
					if(fmt_pos == 'p')
						flags |= SPRINTF_ALT_FLAG;
				
					if(flags & SPRINTF_ALT_FLAG)
					{
						put_in_string(string, ssz, '0', string_pos++);
						
						if(fmt[fmt_pos] == 'X')
							put_in_string(string, ssz, 'X', string_pos++);
						else
							put_in_string(string, ssz, 'x', string_pos++);
					}
				
					calculate_real_padding_hex();
					write_padding();

					for(x=7;x>=0;x--)
					{
						y = arg >> (x << 2);
						y &= 0xf;
						
						if(y>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
						{
							if(y>=0 && y<=9)
								put_in_string(string, ssz, y + '0', string_pos++);
							else if(y>=0xA && y<=0xF)
							{
								if(fmt[fmt_pos] == 'X')
									put_in_string(string, ssz, (y - 0xa) + 'A', string_pos++);
								else
									put_in_string(string, ssz, (y - 0xa) + 'a', string_pos++);
							}
						}
					}
				
					write_neg_padding();

					directive_coming = 0;
				break;
				case 'c': // character
					arg = va_arg(ap, int);
				
					put_in_string(string, ssz, arg & 0xff, string_pos++);
					
					directive_coming = 0;
				break;
				case 's': // string
					argcp = va_arg(ap, char *);
					argcp_tmp = argcp;
				
					if(argcp == NULL)
					{
						// Non standard extension, but supported by Linux and the BSDs.
					
						put_in_string(string, ssz, '(', string_pos++);
						put_in_string(string, ssz, 'n', string_pos++);
						put_in_string(string, ssz, 'u', string_pos++);
						put_in_string(string, ssz, 'l', string_pos++);
						put_in_string(string, ssz, 'l', string_pos++);
						put_in_string(string, ssz, ')', string_pos++);
						
						directive_coming = 0;
						break;
					}
				
					while(*argcp_tmp)
					{
						if(pad_quantity > 0) pad_quantity--;
						argcp_tmp++;
					}
					
					if(!(flags & SPRINTF_NEGFIELD_FLAG))
					{
						while(pad_quantity > 0)
						{
							put_in_string(string,ssz, ' ', string_pos++);
							pad_quantity--;
						}
					}
					
					while(*argcp)
					{
						put_in_string(string, ssz, *argcp, string_pos++);
						argcp++;
					}
					
					if(flags & SPRINTF_NEGFIELD_FLAG)
					{
						while(pad_quantity > 0)
						{
							put_in_string(string,ssz, ' ', string_pos++);
							pad_quantity--;
						}
					}
						
					directive_coming = 0;
				break;
				case 'o': // Octal
					empty_digit = 1;
					arg = va_arg(ap, unsigned int);
				
					for(x=10;x>=0;x--)
					{
						y = arg >> (x * 3);
						y &= 0x7;
						
						if(y>=1)
							empty_digit = 0;
						
						if(empty_digit == 0 || x == 0)
							put_in_string(string, ssz, y + '0', string_pos++);	
					}
				
					directive_coming = 0;
				break;
				case 'n': // Number of characters written
					*(va_arg(ap,unsigned int*)) = string_pos;
					
					directive_coming = 0;
				break;
			//	default
			}
		}
		else
		{
			if(fmt[fmt_pos] == '%')
			{
				directive_coming = 1;
				flags = 0;
				argsize = 2;
				pad_quantity = 0;
				zero_flag_imp = 0;
			}
			else
				put_in_string(string, ssz, fmt[fmt_pos], string_pos++);
		}	
	}
	string[string_pos] = 0;
	return string_pos;	
}

int vsprintf(char *string, char *fmt, va_list ap)
{
	return vsnprintf(string, 0xffffffff, fmt, ap);
}

int sprintf(char *string, char *fmt, ...)
{
	int r;

	va_list ap;

	va_start(ap, fmt);

	r = vsprintf(string, fmt, ap);
	
	va_end(ap);
	
	return r;
}

int snprintf(char *string, unsigned int size, char *fmt, ...)
{
	int r;

	va_list ap;

	va_start(ap, fmt);

	r = vsnprintf(string, size, fmt, ap);
	
	va_end(ap);
	
	return r;
}

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

int abs(int j)
{
	if(j < 0) j*=-1;
	return j;
}

void *memset(void *dst , char c , int n)
{
	char *dstc = (unsigned char*)dst;
	int x;

	for(x = 0; x < n; x++)
		dstc[x] = c;
}

int memcmp(void *b1, void *b2, int n)
{
	int x;
	unsigned char *bp1 = (unsigned char*)b1;
	unsigned char *bp2 = (unsigned char*)b2;
	
	for(x = 0;  x < n; x++)
		if(bp1[x] != bp2[x])
			return (bp1[x] - bp2[x]);
		
	return 0;
}

void *memmove(void *dst, void *src, int len)
{
	void *dst2 = dst;
	
	dst+=len-1;
	src+=len-1;
	
	while(len--)
		*(((unsigned char*)dst--)) = *(((unsigned char*)src--));
		
	return dst2;
}

void *memchr(void *s , int c , int n)
{
	while(n--)
	{
		if(*((unsigned char*)s) == (unsigned char)c)
			return s;
		
		s++;
	}
	
	return NULL;
}

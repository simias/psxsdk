/*
 * PSXSDK Library
 *
 * Free and open source library to develop for the Sony PlayStation
 */
 
#include <psx.h>
#include <stdio.h>

#define IPENDING				*((unsigned int*)0x1f801070)
#define IMASK					*((unsigned int*)0x1f801074)
#define RCNT_COUNT(x)			*((unsigned int*)(0x1f801100 + (x<<4)))
#define RCNT_MODE(x)			*((unsigned int*)(0x1f801104 + (x<<4)))
#define RCNT_TARGET(x)		*((unsigned int*)(0x1f801108 + (x<<4)))

const char *sysromver_unavail = "System ROM Version Unavailable";

unsigned char padread_buf[2][8];

void (*vblank_handler_callback)();

extern int *vblank_handler();

void (*rcnt_handler_callback)();

extern int *rcnt_handler();

unsigned int vblank_queue_buf[4] = {0x0, /* Will contain next interrupt handler in queue */
                                    0x0, /* func1 */
				    (unsigned int)vblank_handler, /* func2 */
				    0x0, /* pad */
				   };
				    
int vblank_handler_set = 0;				   
unsigned int vblank_handler_event_id = 0;

int rcnt_handler_set = 0;
unsigned int rcnt_handler_event_id = 0;				   
				   
void PSX_Init()
{
	unsigned short btnbuf;
	unsigned short btnbuf2;
	
	/* Reinitialize ISO 9660 filesystem driver */
	
	EnterCriticalSection();
	_96_remove();
	ExitCriticalSection();
	
	_96_init();
	
	/* Initialize memory card */
        
        InitCARD(1);

// Start BIOS memory card handling 	
	
        StartCARD();
        _bu_init();

// Stop BIOS memory card handling
// We will start the BIOS memory card handling only when we really need it
// Otherwise they get into conflicts with the low-level pad reading routines
// and everything freezes...
// The conflicts happen because some internal routines for the memory
// card are run by the BIOS at regular time intervals

	StopCARD();

// Pad initialization stuff here...
#warning "Initialize pads even if the BIOS does not do it!!"
	
	printf("PSXSDK testing version !!!\n");
	
	vblank_handler_set = 0;
}

void PSX_ReadPad(unsigned short *padbuf, unsigned short *padbuf2)
{
	int x;
	unsigned char arr[16];
	unsigned short *padbuf_a[2];
	
// Now uses low level pad routines...	
	padbuf_a[0] = padbuf;
	padbuf_a[1] = padbuf2;
		
	for(x = 0; x < 2; x++)
	{
		pad_read_raw(x, arr);
		
		if(arr[2] == 0x5a)
		{
			*padbuf_a[x] = (arr[3]<<8)|arr[4];
			*padbuf_a[x] = ~*padbuf_a[x];
		}
		else
			*padbuf_a[x] = 0;
	}
}

int PSX_GetPadType(unsigned int pad_num)
{
	/*if(pad_num >= 2)
		return PADTYPE_NONE;

	if(padread_buf[pad_num][0] != 0)
		return PADTYPE_NONE;

	return (padread_buf[pad_num][1]>>4)&0xf;*/
}


void PSX_GetSysInfo(struct psx_info *info)
{
	unsigned long i,i2;
	int x;

	info->kernel.version = GetKernelRomVersion();
	
	i = GetKernelDate();
	
/*
 * Convert year from BCD to decimal
 */
	
	i2 = i >> 16;
	
	info->kernel.year = i2 & 0xf;
	info->kernel.year+= ((i2>>4)&0xf)*10;
	info->kernel.year+= ((i2>>8)&0xf)*100;
	info->kernel.year+= ((i2>>12)&0xf)*1000;
	
/*
 * Convert month from BCD to decimal
 */
	i2 = (i >> 8) & 0xff;
	
	info->kernel.month = i2 & 0xf;
	info->kernel.month+= (i2>>4) * 10;	
		
/*
 * Convert day from BCD to decimal
 */
	i2 = i & 0xff;
	
	info->kernel.day = i2 & 0xf;
	info->kernel.day+= (i2>>4) * 10;

/*
 * Unless we receive something in the range >= 1 && <= 16,
 * RAM size will be reported as 2 Megabytes
 */
 
	i = GetRamSize();
	
	if(i == 0 || i > 16)
		info->system.memory = 2<<20; /* 2 Megabytes */
	else
		info->system.memory <<= 20;
}
	


int get_real_file_size(char *name)
{
	struct DIRENTRY dirent_buf;
	
	if(firstfile(name, &dirent_buf) == &dirent_buf)
		return dirent_buf.size;
	else
		return 0;
}

int get_file_size(char *name)
{
	int i = get_real_file_size(name);
	
	if(strncmp(name, "cdrom:", 6) == 0)
	{
		if(i & 0x7ff)
		{
			i += 0x800;
			i &= ~0x7ff;
		}
	}else if(strncmp(name, "bu", 2) == 0)
	{
		if(i & 0x7f)
		{
			i += 0x80;
			i &= ~0x7f;
		}
	}
	
	return i;
}

int SetRCnt(int spec, unsigned short target, unsigned int mode)
{
	spec &= 0xf;
	
	if(spec >= 3)
		return 0;
	
	RCNT_MODE(spec)=0;
	RCNT_TARGET(spec)=target;
	RCNT_MODE(spec)=mode;
	
	return 1;
}

int GetRCnt(int spec)
{
	spec &= 0xf;
	
	if(spec >= 4)
		return -1;
	
	return (RCNT_COUNT(spec) & 0xffff);
}

int StartRCnt(int spec)
{
	spec &= 0xf;
	
	if(spec >= 3)
		return 0;
	
	IMASK |= 1 << (spec + 4);
	
	return 1;
}

int StopRCnt(int spec)
{
	spec &= 0xf;
	
	if(spec >= 3)
		return 0;
	
	IMASK ^= 1 << (spec + 4);
	
	return 1;
}
	
void SetVBlankHandler(void (*callback)())
{
	if(vblank_handler_set == 1)
	{
		EnterCriticalSection();
		
		vblank_handler_callback = callback;
	
		ExitCriticalSection();
		
		return;
	}
	
// Enter critical section
	
	EnterCriticalSection();
	
	IMASK|=1;
	
	vblank_handler_event_id = OpenEvent(RCntCNT3, 2, 0x1000, vblank_handler);
	EnableEvent(vblank_handler_event_id);
	
	vblank_handler_callback = callback;
	vblank_handler_set = 1;

// Exit critical section
	
	ExitCriticalSection();
}

void RemoveVBlankHandler()
{
	DisableEvent(vblank_handler_event_id);
	CloseEvent(vblank_handler_event_id);
	
	vblank_handler_set = 0;
}

void SetRCntHandler(void (*callback)(), int spec, unsigned short target)
{
	if(rcnt_handler_set == 1)
	{
		EnterCriticalSection();
		
		rcnt_handler_callback = callback;
		
		ExitCriticalSection();
		
		return;
	}
	
// Enter critical section
	
	SetRCnt(spec, target, RCntIntr | 0x08 | 0x10 | 0x40);
	StartRCnt(spec);
	
	EnterCriticalSection();
	rcnt_handler_event_id = OpenEvent(spec, 2, 0x1000, rcnt_handler);
	EnableEvent(rcnt_handler_event_id);
	
	rcnt_handler_callback = callback;
	rcnt_handler_set = 1;

// Exit critical section
	
	ExitCriticalSection();
}

const char *GetSystemRomVersion()
{
// Get pointer to zero-terminated string containing System ROM Version which is embedded in
// most PlayStation BIOSes.

// If getting the pointer is not possible, a pointer to a string saying "System ROM Unavailable" is returned.
	
	int x;
	
	for(x = 0x7ffee; x >= 0; x--)
		if(memcmp("System ROM Version", (void*)(0xbfc00000 + x), 18) == 0)
			return (char*)(0xbfc00000 + x);
	
	return sysromver_unavail;
}

/*
 * PSXSDK: Bios functions
 */

#ifndef _PSXBIOS_H
#define _PSXBIOS_H

/* Joypad functions */

extern void PAD_init(unsigned long mode, unsigned long *pad_buf);
extern int PAD_dr();

/* ROM information functions */

/* 
 * GetKernelDate() returns PSX kernel date in 0xYYYYMMDD BCD format.
 */
 
unsigned long GetKernelDate();

/*
 * GetKernelRomVersion() returns a pointer to a zero-terminated
 * string which contains the kernel ROM version.
 */

const char *GetKernelRomVersion();

/*
 * GetRamSize() should return size of RAM in bytes.
 * It doesn't seem to work most times. On SCPH1001, it returns 0.
 * On SCPH1000, it returns 2 (which is the number of megabytes of RAM
 * the PSX has.)
 */

unsigned int GetRamSize();

/* Event functions */

unsigned int openevent(unsigned int class, unsigned int spec, unsigned int mode, void (*func)());
unsigned int enableevent(unsigned int eventid);

/* Interrupt/Exception functions */

/*void Exception();*/
void EnterCriticalSection();
void ExitCriticalSection();

void SysEnqIntRP(int index, unsigned int *buf);
void SysDeqIntRP(int index, unsigned int *buf);

void ResetEntryInt();



struct DIRENTRY
{
	char name[20]; // Filename
	int attr; // Attributes
	int size; // File size in bytes
	struct DIRENTRY *next; // Pointer to next file entry
	char system[8]; // System reserved
};

/*
 * firstfile() gets information about the first file which
 * matches the pattern. ? and * wildcards can be used.
 * Characters after * are ignored.
 */

struct DIRENTRY *firstfile(char *name, struct DIRENTRY *dirent);

/* 
 * get_file_size() gets the file size of the file named "name".
 * It is actually just a wrapper around firstfile.
 * It rounds the file size to the block size.
 */

int get_file_size(char *name);

/*
 * get_real_file_size() is like get_file_size() but doesn't round
 * the file size to the block size.
 */
 
int get_real_file_size(char *name);

void InitHeap(void *block , int size);
void FlushCache();

void SetVBlankHandler(void (*h)());
void RemoveVBlankHandler();

void SetRCntHandler(void (*callback)(), int spec, unsigned short target);

// OpenEvent() opens an event, and returns its identifier
// Must be executed in a critical section

int OpenEvent(
	int desc, // Cause descriptor
	int spec, // Event type
	int mode, // Mode
	int *(*func)() // Pointer to callback function
);

// EnableEvent() enables an event by its identifier returned by OpenEvent()

int EnableEvent(unsigned int event);

// CloseEvent() closes and event by its identifier

int CloseEvent(unsigned int event);

// DisableEvent() disables an event by its identifier

int DisableEvent(unsigned int event);

// DeliverEvent() generates an event. This must be executed in a critical section.
// If the event to deliver is set to generate an interrupt, the handler function is called.

int DeliverEvent(unsigned int ev1, // Cause descriptor
			  int ev2); // Event class
			  
// TestEvent() checks if the event specified by its identifier has occured
// It returns 1 if the event has occured, 0 if it has not

int TestEvent(unsigned int event);

// WaitEvent() waits until the event specified by identifier occurs.
// It returns 1 if it succeds, 0 if there is a failure.

int WaitEvent(unsigned int event);

#endif

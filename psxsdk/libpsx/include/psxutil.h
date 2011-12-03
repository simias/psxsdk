// psxutil.h

#ifndef _PSXUTIL_H
#define _PSXUTIL_H

// PSX_GetButtonName() takes three arguments.
// button is a button bitmask as returned by PSX_ReadPad()
// out is a pointer to a string, to which the button names will be output
// out_len is the maximum length of the string, to avoid buffer overflows.

// If more than one button is found in the bitmask, it is specified by the symbol &
// followed by the other button name and so on...

void *PSX_GetButtonName(unsigned short button, unsigned char *out, unsigned int out_len);

#endif

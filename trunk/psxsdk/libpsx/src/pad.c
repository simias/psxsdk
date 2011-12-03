#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <psx.h>

/*
 * Flags taken from PCSX
 */

// Status Flags
#define TX_RDY		0x0001
#define RX_RDY		0x0002
#define TX_EMPTY	0x0004
#define PARITY_ERR	0x0008
#define RX_OVERRUN	0x0010
#define FRAMING_ERR	0x0020
#define SYNC_DETECT	0x0040
#define DSR			0x0080
#define CTS			0x0100
#define IRQ			0x0200

// Control Flags
#define TX_PERM		0x0001
#define DTR			0x0002
#define RX_PERM		0x0004
#define BREAK		0x0008
#define RESET_ERR	0x0010
#define RTS			0x0020
#define SIO_RESET	0x0040

/*
    from BlackBag/Nagra PSX

    0x1f801040 - unsigned char data;
    0x1f801044 - unsigned short status;
    0x1f80104a - unsigned short cntl;
    0x1f80104e - unsigned short baud;
*/

#define SIO_DATA(x)	*((unsigned char*)(0x1f801040 + (x<<4)))
#define SIO_STATUS(x)	*((unsigned short*)(0x1f801044 + (x<<4)))
#define SIO_CTRL(x)	*((unsigned short*)(0x1f80104a + (x<<4)))
#define SIO_BAUD(x)	*((unsigned short*)(0x1f80104e + (x<<4)))

unsigned char readpad_vibrations[4][2];
int querypad_rxrdy = 1;

void QueryPAD(int pad_n, unsigned char *in, unsigned char *out, int len)
{
// This was based on a C conversion from ASM code by bITmASTER	
	
	unsigned short sbuf;
	int x;
	int y;
	
	SIO_CTRL(0) = 0x2; // Send DTR to Control Register
	
	if(pad_n == 1)
		SIO_CTRL(0) = 0x3003; // Select PAD 1
	else
		SIO_CTRL(0) = 0x1003; // b1 = PAD 0, select PAD 0
	
	while(!(SIO_CTRL(0) & 1)); // Wait for TX_PERM ???
	
	for(x = 0; x < len; x++)
	{
		SIO_DATA(0) = *in;
		in++;
#warning "This delay occasionally might give problems to the code of games"
#warning "and so it will have to be removed..."
		//for(y=0;y<400;y++); // Delay ...
		y = 0;
		
		if(querypad_rxrdy)
		{
			while(!(SIO_STATUS(0) & RX_RDY))
			{
				y++;
				if(y == 1000)
				{
					querypad_rxrdy = 0;
					break;
				}
			}
		}
					
		sbuf = SIO_CTRL(0) | 0x10; // RESET_ERR ???
		SIO_CTRL(0) = sbuf;
		
		// wait status ???
		if(out == NULL)
			y = SIO_DATA(0);
		else
		{
			*out = SIO_DATA(0);
			out++;
		}
			
		while(!(SIO_CTRL(0) & 2)); // wait for DTR bit set ???
	}
	
	SIO_CTRL(0) = 0;
}

void pad_read_raw(int pad_n, unsigned char *arr)
{
	// arr must be at least 16 bytes long...
	
	unsigned char pad_cmd[16] = {1,0x42,0,0,0};
	
	pad_cmd[3] = readpad_vibrations[pad_n][0];
	pad_cmd[4] = readpad_vibrations[pad_n][1];
	
	QueryPAD(pad_n, pad_cmd, arr, sizeof(pad_cmd));
}

void pad_escape_mode(int pad_n, int enable)
{
	unsigned char pad_cmd[] = {1,0x43,0,enable?1:0,0};
	
	QueryPAD(pad_n, pad_cmd, NULL, sizeof(pad_cmd));
}

void pad_enable_vibration(int pad_n)
{
	unsigned char pad_cmd[] = {1, 0x4d, 0, 0, 1, 0xff, 0xff, 0xff, 0xff};
	
	pad_escape_mode(pad_n, 1); // Enter escape / configuration mode
	QueryPAD(pad_n, pad_cmd, NULL, sizeof(pad_cmd));
	pad_escape_mode(pad_n, 0); // Exit escape / configuration mode
}

void pad_set_vibration(int pad_n, unsigned char small, unsigned char big)
{
	readpad_vibrations[pad_n][0] = small;
	readpad_vibrations[pad_n][1] = big;
}

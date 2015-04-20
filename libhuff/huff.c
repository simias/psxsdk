// Huffman decompression library for PSXSDK

// Huffman decompression code adapted from huff program
// by Joe Wingbermuehle

#include <psx.h>
#include "huff.h"

#define BUFFER_SIZE	1024

unsigned char libhuff_inBuffer[BUFFER_SIZE];
unsigned char libhuff_outBuffer[BUFFER_SIZE];

struct CodeNode {
	unsigned char value;
	unsigned long code;
	unsigned int codeSize;
};

struct CodeNode libhuff_codes[256];
unsigned int libhuff_codesUsed;

// Decompress_Mem returns size of uncompressed data if SizeLimit >= UncompressedDataSize,
// otherwise if UncompressedDataSize > SizeLimit, 0 is returned

// SizeLimit is the maximum space available for decompressed data

unsigned int huff_decompress(void *dst, void *src, int sizeLimit) {
	int x, y;
	unsigned int dataSize;
	unsigned long mask, maskSize;
	unsigned char ch;
	int offset;
	int last;

	int ib, ob;
	int src_pos = 8;
	int dst_pos = 0;
	unsigned char *srcc = (unsigned char*)src;
	unsigned char *dstc = (unsigned char*)dst;

	libhuff_codesUsed = *((unsigned int*)src);
	dataSize = *((unsigned int*)src + 1);

	if(dataSize > sizeLimit)
		return 0;

	for(x = 0; x < libhuff_codesUsed; x++) {
		libhuff_codes[x].value = srcc[src_pos++];
		libhuff_codes[x].codeSize = srcc[src_pos++] + 1;
	}

	offset = 7;
	ch = 0;
	for(x = 0; x < libhuff_codesUsed; x++) {
		libhuff_codes[x].code = 0;
		for(y = libhuff_codes[x].codeSize - 1; y >= 0; y--) {
			if(offset == 7) {
				ch = srcc[src_pos++];
			}
			libhuff_codes[x].code |= ((ch >> offset) & 1) << y;
			offset = (offset - 1) & 7;
		}
	}

	maskSize = 0;
	mask = 0;
	offset = 7;
	last = 0;
	y = 0;
	x = 0;
	ib = BUFFER_SIZE;
	ob = 0;
	for(;;) {
		if(offset == 7) {
			if(ib >= BUFFER_SIZE) {
				memcpy(libhuff_inBuffer, &srcc[src_pos], BUFFER_SIZE);
				src_pos += BUFFER_SIZE;

				ib = 0;
			}
			ch = libhuff_inBuffer[ib++];
		}
		mask <<= 1;
		mask |= (ch >> offset) & 1;
		++maskSize;
		offset = (offset - 1) & 7;

		while(libhuff_codes[y].codeSize < maskSize) ++y;
		while(libhuff_codes[y].codeSize == maskSize) {
			if(libhuff_codes[y].code == mask) {
				if(ob >= BUFFER_SIZE) {
					memcpy(&dstc[dst_pos], libhuff_outBuffer, BUFFER_SIZE);
					dst_pos += BUFFER_SIZE;
					ob = 0;
				}
				libhuff_outBuffer[ob++] = libhuff_codes[y].value;
				++x;
				if(x >= dataSize) {
					memcpy(&dstc[dst_pos], libhuff_outBuffer, ob);
					dst_pos += ob;
					return dataSize;
				}
				mask = 0;
				maskSize = 0;
				y = 0;
				break;
			}
			++y;
		}
	}

	return dataSize;
}


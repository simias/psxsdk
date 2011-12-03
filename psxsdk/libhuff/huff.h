/*
 * huff.h
 *
 * Huffman decompression routines
 * 
 * libhuff
 */

#ifndef _HUFF_H
#define _HUFF_H

// Decompress_Mem returns size of uncompressed data if SizeLimit >= UncompressedDataSize,
// otherwise if UncompressedDataSize > SizeLimit, 0 is returned

// SizeLimit is the maximum space available for decompressed data

unsigned int huff_decompress(void *dst, void *src, int sizeLimit);

#endif

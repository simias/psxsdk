
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE	1024

struct TreeNode {
	unsigned char value;
	unsigned long freq;
	struct TreeNode *left;
	struct TreeNode *right;
};

struct CodeNode {
	unsigned char value;
	unsigned long code;
	unsigned int codeSize;
};

unsigned long table[256];
unsigned char value[256];
unsigned long freq[256];
int tableSize;

struct CodeNode codes[256];
unsigned int codesUsed;

FILE *inFile;
unsigned long inputSize;

FILE *outFile;

struct TreeNode *root;

char inBuffer[BUFFER_SIZE];
char outBuffer[BUFFER_SIZE];

void CreateTable();
void SiftHeap(struct TreeNode**, unsigned int, unsigned int);
void SortTrees(struct TreeNode**, unsigned int);
void DestroyTree(struct TreeNode *);
void CreateTree();
void GenerateCodes(struct TreeNode*, unsigned long, unsigned long);
void SortCodes();
void Compress();
void Decompress();
void DisplayHelp();
int main(int, char**);

//////////////////////////////////////////////////////////////////////////
// Create the character frequency table for the file
//////////////////////////////////////////////////////////////////////////
void CreateTable() {
	int x, y;
	unsigned long maxCount;
	unsigned char maxIndex;
	unsigned char ch;

	for(x = 0; x < 256; x++) {
		value[x] = 0;
		freq[x] = 0;
	}

	rewind(inFile);
	tableSize = 0;
	for(;;) {
		ch = fgetc(inFile);
		if(feof(inFile)) {
			break;
		}
		++inputSize;
		if(table[ch] == 0) {
			++tableSize;
		}
		++table[ch];
	}

	y = tableSize;
	do {
		--y;
		maxIndex = 0;
		maxCount = 0;
		for(x = 0; x < 256 ; x++) {
			if(table[x] > maxCount) {
				maxIndex = x;
				maxCount = table[(unsigned int)maxIndex];
			}
		}
		freq[y] = maxCount;
		value[y] = maxIndex;
		table[(unsigned int)maxIndex] = 0;
	} while(y > 0);
}

//////////////////////////////////////////////////////////////////////////
// Max-heapify (for sorting)
//////////////////////////////////////////////////////////////////////////
void SiftHeap(struct TreeNode **trees, unsigned int x, unsigned int size) {
	struct TreeNode *root;
	int finished;
	unsigned int y;

	root = trees[x - 1];
	y = x << 1;

	finished = (y > size);
	while(!finished) {
		if(y < size) {
			if(trees[y + 1 - 1]->freq > trees[y - 1]->freq) {
				++y;
			}
		}
		if(trees[y - 1]->freq > root->freq) {
			trees[x - 1] = trees[y - 1];
			x = y;
			y = x << 1;
			finished = (y > size);
		} else {
			finished = 1;
		}
	}
	trees[x - 1] = root;
}

//////////////////////////////////////////////////////////////////////////
// Sort the trees in ascending order by frequency
//////////////////////////////////////////////////////////////////////////
void SortTrees(struct TreeNode **trees, unsigned int num) {
	struct TreeNode *temp;
	unsigned int x;

	for(x = num >> 1; x > 1; x--) {
		SiftHeap(trees, x, num);
	}

	for(x = num; x > 1; x--) {
		SiftHeap(trees, 1, x);
		temp = trees[1 - 1];
		trees[1 - 1] = trees[x - 1];
		trees[x - 1] = temp;
	}
}

//////////////////////////////////////////////////////////////////////////
// Release the memory used by the tree
//////////////////////////////////////////////////////////////////////////
void DestroyTree(struct TreeNode *ptr) {
	if(ptr->right) {
		DestroyTree(ptr->right);
	}
	if(ptr->left) {
		DestroyTree(ptr->left);
	}
	free(ptr);
}

//////////////////////////////////////////////////////////////////////////
// Create the huffman coding tree
//////////////////////////////////////////////////////////////////////////
void CreateTree() {
	struct TreeNode *ptr;
	struct TreeNode *trees[256];
	int x, y;

	for(x = 0; x < tableSize; x++) {
		trees[x] = malloc(sizeof(struct TreeNode));
		trees[x]->right = 0;
		trees[x]->left = 0;
		trees[x]->value = value[x];
		trees[x]->freq = freq[x];
	}

	y = tableSize;
	while(y > 1) {
		// Combine two smallest nodes into a tree
		ptr = malloc(sizeof(struct TreeNode));
		ptr->right = trees[0];
		ptr->left = trees[1];
		ptr->freq = trees[0]->freq + trees[1]->freq;
		ptr->value = 0;
		trees[0] = ptr;
		for(x = 1; x < y - 1; x++) {
			trees[x] = trees[x + 1];
		}
		trees[y] = 0;
		SortTrees(trees, y - 1);	// account for the zero
		--y;
	}

	root = trees[0];
}

//////////////////////////////////////////////////////////////////////////
// Generate code table from the tree
//////////////////////////////////////////////////////////////////////////
void GenerateCodes(struct TreeNode *ptr, unsigned long code,
	unsigned long codeSize) {
	if(ptr->right) {
		GenerateCodes(ptr->right, (code << 1), codeSize + 1);
		GenerateCodes(ptr->left, (code << 1) | 1, codeSize + 1);
	} else {
		codes[codesUsed].value = ptr->value;
		codes[codesUsed].code = code;
		codes[codesUsed].codeSize = codeSize;
		++codesUsed;
	}
}

//////////////////////////////////////////////////////////////////////////
// Sort the codes in ascending order by size
// Used for compression
//////////////////////////////////////////////////////////////////////////
void SortCodes() {
	int x, y;
	struct CodeNode temp;

	for(x = 0; x < codesUsed; x++) {
		for(y = x; y < codesUsed; y++) {
			if(codes[x].codeSize > codes[y].codeSize) {
				temp = codes[x];
				codes[x] = codes[y];
				codes[y] = temp;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Compress the file
//////////////////////////////////////////////////////////////////////////
void Compress() {
	int x, y;
	unsigned char temp;
	unsigned char ch;
	int offset;
	int ib, ob;
	int inBufferSize;

	CreateTable();
	CreateTree();
	codesUsed = 0;
	GenerateCodes(root, 0, 0);
	DestroyTree(root);
	SortCodes();

// [nextvolume]: Values are now saved as 32-bit little endian instead of ASCII numbers	
	
	fputc(codesUsed & 0xff, outFile);
	fputc((codesUsed >> 8) & 0xff, outFile);
	fputc((codesUsed >> 16) & 0xff, outFile);
	fputc((codesUsed >> 24) & 0xff, outFile);
	
	fputc(inputSize & 0xff, outFile);
	fputc((inputSize >> 8) & 0xff, outFile);
	fputc((inputSize >> 16) & 0xff, outFile);
	fputc((inputSize >> 24) & 0xff, outFile);

	for(x = 0; x < codesUsed; x++) {
		fputc(codes[x].value, outFile);
		fputc(codes[x].codeSize - 1, outFile);
	}

	offset = 7;
	temp = 0;
	for(x = 0; x < codesUsed; x++) {
		for(y = codes[x].codeSize - 1; y >= 0; y--) {
			temp |= ((codes[x].code >> y) & 1) << offset;
			--offset;
			if(offset < 0) {
				fputc(temp, outFile);
				offset = 7;
				temp = 0;
			}
		}
	}
	if(offset != 7) {
		fputc(temp, outFile);
	}

	offset = 7;
	temp = 0;
	rewind(inFile);
	ib = BUFFER_SIZE;
	ob = 0;
	inBufferSize = 0;
	for(;;) {
		if(ib >= BUFFER_SIZE) {
			inBufferSize = fread(inBuffer, sizeof(char),
				BUFFER_SIZE, inFile);
			ib = 0;
		}
		if(ib >= inBufferSize) {
			break;
		}
		ch = inBuffer[ib++];
		for(x = 0; x < codesUsed; x++) {
			if(ch == codes[x].value) {
				break;
			}
		}
		for(y = codes[x].codeSize - 1; y >= 0; y--) {
			temp |= ((codes[x].code >> y) & 1) << offset;
			--offset;
			if(offset < 0) {
				outBuffer[ob++] = temp;
				if(ob >= BUFFER_SIZE) {
					fwrite(outBuffer, sizeof(char),
						BUFFER_SIZE, outFile);
					ob = 0;
				}
				temp = 0;
				offset = 7;
			}
		}
	}
	if(offset != 7) {
		outBuffer[ob++] = temp;
	}
	if(ob) {
		fwrite(outBuffer, sizeof(char), ob, outFile);
	}
}

//////////////////////////////////////////////////////////////////////////
// Decompress the file
//////////////////////////////////////////////////////////////////////////
void Decompress() {
	int x, y;
	unsigned int dataSize;
	unsigned long mask, maskSize;
	unsigned char ch;
	int offset;
	int last;

	int ib, ob;

	ib = fgetc(inFile);
	codesUsed = ib;
	ib = fgetc(inFile);
	codesUsed |= ib << 8;
	ib = fgetc(inFile);
	codesUsed |= ib << 16;
	ib = fgetc(inFile);
	codesUsed |= ib << 24;

	ib = fgetc(inFile);
	dataSize = ib;
	ib = fgetc(inFile);
	dataSize |= ib << 8;
	ib = fgetc(inFile);
	dataSize |= ib << 16;
	ib = fgetc(inFile);
	dataSize |= ib << 24;

	for(x = 0; x < codesUsed; x++) {
		codes[x].value = fgetc(inFile);
		codes[x].codeSize = fgetc(inFile) + 1;
	}

	offset = 7;
	ch = 0;
	for(x = 0; x < codesUsed; x++) {
		codes[x].code = 0;
		for(y = codes[x].codeSize - 1; y >= 0; y--) {
			if(offset == 7) {
				ch = fgetc(inFile);
			}
			codes[x].code |= ((ch >> offset) & 1) << y;
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
				fread(inBuffer, sizeof(char), BUFFER_SIZE,
					inFile);
				ib = 0;
			}
			ch = inBuffer[ib++];
		}
		mask <<= 1;
		mask |= (ch >> offset) & 1;
		++maskSize;
		offset = (offset - 1) & 7;

		while(codes[y].codeSize < maskSize) ++y;
		while(codes[y].codeSize == maskSize) {
			if(codes[y].code == mask) {
				if(ob >= BUFFER_SIZE) {
					fwrite(outBuffer, sizeof(char),
						BUFFER_SIZE, outFile);
					ob = 0;
				}
				outBuffer[ob++] = codes[y].value;
				++x;
				if(x >= dataSize) {
					fwrite(outBuffer, sizeof(char),
						ob, outFile);
					return;
				}
				mask = 0;
				maskSize = 0;
				y = 0;
				break;
			}
			++y;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Display usage
//////////////////////////////////////////////////////////////////////////
void DisplayHelp() {
	printf("Huffman compressor for PSXSDK\n");
	printf("Original version by Joe Wingbermuehle\n");
	printf("usage: huff [options] file\n");
	printf("options:\n");
	printf("\t-\t\tUse stdin for input\n");
	printf("\t-c\t\tCompress/Uncompress to stdout\n");
	printf("\t-k\t\tKeep old file\n");
	printf("\t-u\t\tUncompress\n");
}

//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
	char *inName;
	char *outName;
	int x, y;
	int error;
	char uncompress;
	char keep;
	double savings;

	if(argc < 2) {
		DisplayHelp();
		exit(1);
	}

	uncompress = 0;
	keep = 0;
	inName = 0;
	outName = 0;
	outFile = 0;
	inFile = 0;
	for(x = 1; x < argc; x++) {
		if(argv[x][0] == '-') {
			switch(argv[x][1]) {
			case 'u':
				uncompress = 1;
				break;
			case 'c':
				outFile = stdout;
				keep = 1;
				break;
			case 'k':
				keep = 1;
				break;
			case 0:
				inFile = stdout;
				break;
			default:
				DisplayHelp();
				exit(1);
			}
		} else if(!inName && !inFile) {
			inName = malloc(strlen(argv[x]) + 1);
			strcpy(inName, argv[x]);
		} else {
			printf("unrecognized option: %s\n", argv[x]);
			DisplayHelp();
			exit(1);
		}
	}

	if(!inFile) {
		inFile = fopen(inName, "r");
		if(!inFile) {
			printf("error: could not open input\n");
			exit(1);
		}
	}

	if(!uncompress) {
		outName = malloc(strlen(inName) + 4);
		strcpy(outName, inName);
		strcat(outName, ".jh");
	} else {
		error = 0;
		if(strlen(inName) < 5) {
			error = 1;
		} else {
			y = strlen(inName) - 3;
			for(x = 0; x < 4; x++) {
				if(inName[x + y] != ".jh"[x]) {
					error = 1;
					break;
				}
			}
		}
		if(error) {
			printf("bad file extension\n");
			DisplayHelp();
			exit(1);
		}
		outName = malloc(strlen(inName) + 1);
		strcpy(outName, inName);
		if(strlen(outName) > 4) {
			outName[strlen(outName) - 3] = 0;
		}
	}

	if(!outFile) {
		outFile = fopen(outName, "w");
		if(!outFile) {
			printf("error: could not open output\n");
			exit(1);
		}
	}

	if(uncompress) {
		Decompress();
	} else {
		Compress();
		savings = (double)inputSize - (double)ftell(outFile);
		savings /= (double)inputSize;
		savings *= 100.00;
		printf("savings: %.2f%%\n", savings);
	}

	if(inFile != stdin) {
		fclose(inFile);
		if(!keep) {
			remove(inName);
		}
	}
	if(outFile != stdout) {
		fclose(outFile);
	}
	exit(0);
}



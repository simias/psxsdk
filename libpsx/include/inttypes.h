/* inttypes.h */

#ifndef _INTTYPES_H
#define _INTTYPES_H

#include <stdint.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef unsigned long long uintmax_t;
typedef unsigned int uintptr_t;

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;
typedef long long int_least64_t;

typedef char int_fast8_t;
typedef short int_fast16_t;
typedef int int_fast32_t;
typedef long long int_fast64_t;

typedef long long intmax_t;

typedef long intptr_t;

#define PRIu8					"hhu"
#define PRIu16				"hu"
#define PRIu32				"u"
#define PRIu64				"llu"

#define PRIs8					"hhd"
#define PRIs16				"hd"
#define PRIs32				"d"
#define PRIs64				"lld"

#define PRIuLEAST8			"hhu"
#define PRIuLEAST16			"hu"
#define PRIuLEAST32			"u"
#define PRIuLEAST64			"llu"

#define PRIuFAST8				"hhu"
#define PRIuFAST16			"hu"
#define PRIuFAST32			"u"
#define PRIuFAST64			"llu"

#define PRIuMAX				"llu"
#define PRIuPTR				"lu"

#endif

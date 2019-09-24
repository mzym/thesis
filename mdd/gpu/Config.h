/* (c) Mikhail Zymbler */

#ifndef CONFIG_H
#define CONFIG_H

#include <malloc.h>

/* Algorithm params */
// in the future there can be param k - number of discord

/* Types and defines */

// Types

// An item
typedef float item_t;
// Time series
typedef item_t* series_t;
// Different matrixes: distance and subsequences
typedef item_t** matrix_t;

// for SAX representation of time series
typedef char symbol;
typedef symbol* word;

// Memory and alignment

#ifdef __x86_64__
    #define MY_ASSUME_ALIGNED(x) ((void)0)
#else
    #define MY_ASSUME_ALIGNED(x)  __assume_aligned((x),ALIGN_SIZE)
#endif

// Size of alignment for all the data thru the project
#define ALIGN_SIZE	(64)

// Memory allocation with alignment
#ifdef __x86_64__
    #define __align_malloc(cnt)	malloc((cnt))
#else
    #define __align_malloc(cnt)	_mm_malloc((cnt), ALIGN_SIZE)
#endif

// Free memory allocated with alignment 
#define __align_free(ptr)	_mm_free((ptr))

// Math
#define POS_INF (1e20)
#define NEG_INF -(POS_INF)

#define MAX_LONG (9999999999)

#endif

/*
Parallel Dynamic Itemset Counting.
System functions and constants for the algorithm.

(c) 2016 Mikhail Zymbler
*/
#ifndef SYSTEM_H
#define SYSTEM_H

#include <malloc.h>

// Size of alignment for all the data thru the project
#define ALIGN_SIZE	(64)

// Memory allocation with alignment 
#define _align_malloc(cnt)	_mm_malloc((cnt), ALIGN_SIZE)

// Free memory allocated with alignment 
#define _align_free(ptr)	_mm_free((ptr))

// Alignment of a static array
#ifdef _WIN64
#define _align_array(type, var, len)	__declspec(align(ALIGN_SIZE)) type var[len]
#else
#define _align_array(type, var, len)	type var[len] __attribute__((aligned(ALIGN_SIZE)))
#endif

// Force compiler to align a variable
#ifdef _WIN64
#define __ASSUME_ALIGNED(var, len)	
#else
#define __ASSUME_ALIGNED(var, len)	__assume_aligned(var, len)
#endif

#endif

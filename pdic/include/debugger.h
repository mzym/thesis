/*
Parallel Dynamic Itemset Counting.
Simple debugger.

(c) 2016 Mikhail Zymbler
*/

#ifndef DEBUGGER_H
#define DEBUGGER_H

#define NDEBUG

#include <stdio.h>
#include <assert.h>
#include <omp.h>
#include "bitmap.h"

#ifdef _WIN64
#include <intrin.h> 
#else
#include "immintrin.h"
#endif

// Output a message
#ifdef NDEBUG
#define PRINT(msg, ...)
#else
#define DBG_THREAD	(0)
#define PRINT(msg, ...) do { \
	if (omp_get_thread_num() == DBG_THREAD) { \
		printf(msg, ##__VA_ARGS__); \
		fflush(stdout); \
	} \
} while (0);
#endif

// Output START and FINISH messages
#ifdef NDEBUG
#define START(name, ...)
#define FINISH(name, ...)
#else
#define START(name, ...) PRINT("\nSTART:\t" name "\n", ##__VA_ARGS__)
#define FINISH(name, ...) PRINT("FINISH:\t" name "\n", ##__VA_ARGS__)
#endif

// Output a bit mask as ``mask==item1|item2|...''. Note that while output numbering of items (i.e. bits) starts from 1 instead of 0.
#ifdef NDEBUG
#define PRINT_BITMASK(bitmask)	
#else
#define PRINT_BITMASK(bitmask)	do { \
	for (unsigned long long idx = 0; idx < TRANSACTION_BITMASK_LEN; idx++) { \
		PRINT("%llu==|", bitmask[idx]); \
		if (bitmask[idx] == 0) \
			PRINT("|") \
		else \
			for (unsigned int b = 0; b < 64; b++) { \
				if (isbitset(bitmask[idx], b)) { \
					PRINT("%llu|", idx*64 + b + 1); \
				} \
			} \
		PRINT("\t"); \
	} \
	PRINT("\t"); \
} while (0);
#endif

// Output a bitmap. Note that while output numbering of elements starts from 1 instead of 0.
#ifdef NDEBUG
#define PRINT_BITMAP(msg)
#else
#define PRINT_BITMAP(msg) do { \
	PRINT("%s\n", msg); \
	for (unsigned long long t = 0; t < n; t++) { \
		PRINT("BITMAP[%llu]:\t", t + 1); \
		PRINT_BITMASK(BITMAP[t]); \
		PRINT("\n"); \
	} \
} while (0);
#endif

// Output a shape
#ifdef NDEBUG
#define PRINT_SHAPE(shape)
#else
#define PRINT_SHAPE(shape) (shape == BOX ? "BOX" : (shape == CIRCLE ? "CIRCLE" : (shape == NIL ? "NIL" : "????")))
#endif

// Count bits in tidbitmask
#ifdef _WIN64
#define POPCNT(bitmask)	__popcnt64(bitmask)
#else
#define POPCNT(bitmask)	_mm_countbits_64(bitmask)
#endif

// Output an itemset. 
// At the same time check if itemset's `k' attribute is equal to number of bits in `bitmask' attribute set to 1.
#ifdef NDEBUG
#define PRINT_ITEMSET(itemset)
#else
#define PRINT_ITEMSET(itemset) do { \
	PRINT("{mask=="); \
	PRINT_BITMASK(itemset.bitmask); \
	PRINT("k==%lu\tstop==%lu\tsupp==%llu\tshape==%s}\n", itemset.k, itemset.stopno, itemset.support, PRINT_SHAPE(itemset.shape)); \
	unsigned long long bitcnt = 0; \
	for (unsigned long long d = 0; d < TRANSACTION_BITMASK_LEN; d++) { \
		bitcnt += POPCNT(itemset.bitmask[d]); \
	} \
	assert(bitcnt == itemset.k); \
} while (0);
#endif

// Output a set of itemsets. Note that while output numbering of elements starts from 1 instead of 0.
#ifdef NDEBUG
#define PRINT_VECTOR(msg, name, vector, myshape)
#else
#define PRINT_VECTOR(msg, name, vector, myshape) do { \
	unsigned long long vecsize = 0; \
	PRINT("\n%s\n", msg); \
	for (unsigned long long v = 0; v < vector.size(); v++) \
		if (vector[v].shape == myshape) { \
			vecsize++; \
			PRINT("%s[%llu]:\t", name, v + 1); \
			PRINT_ITEMSET(vector[v]); \
		} \
	if (vecsize == 0) \
		PRINT("%s is empty!\n", msg); \
} while (0);
#endif

#ifdef NDEBUG
#define PRINT_VECTOR_ALL(msg, name, vector)
#else
#define PRINT_VECTOR_ALL(msg, name, vector)	do { \
	PRINT("\n%s\n", msg); \
	for (unsigned long long c = 0; c < vector.size(); c++) { \
		PRINT("%s[%llu]:\t", name, c + 1); \
		PRINT_ITEMSET(vector[c]); \
	} \
	if (vector.size() == 0) { \
		PRINT("%s is empty!\n", msg); \
	} \
} while (0);
#endif

// Output key sets of itemsets
#ifdef NDEBUG
#define PRINT_DASHED_CIRCLE(msg)
#define PRINT_DASHED_BOX(msg)
#define PRINT_SOLID_CIRCLE(msg)
#define PRINT_SOLID_BOX(msg)
#define PRINT_CANDIDATES(msg)
#else
#define PRINT_DASHED_CIRCLE(msg)	PRINT_VECTOR(msg, "DC", DASHED, CIRCLE)
#define PRINT_DASHED_BOX(msg)	PRINT_VECTOR(msg, "DB", DASHED, BOX)
#define PRINT_SOLID_CIRCLE(msg)	PRINT_VECTOR(msg, "SC", SOLID, CIRCLE)
#define PRINT_SOLID_BOX(msg)	PRINT_VECTOR(msg, "SB", SOLID, BOX)
#define PRINT_CANDIDATES(msg)	PRINT_VECTOR_ALL(msg, "CAND", CAND)
#endif

#endif

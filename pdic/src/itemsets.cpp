/*
Parallel Dynamic Itemset Counting.
Implementation of the operations with itemsets.

(c) 2016 Mikhail Zymbler
*/

#include "itemsets.h"
#include "debugger.h"
#include "system.h"
#include <vector>

// Count bits in bitmask
#ifdef _WIN64
#define POPCNT(bitmask)	__popcnt64(bitmask)
#else
#define POPCNT(bitmask)	_mm_countbits_64(bitmask)
#endif


// Returns true if an itemset's shape is NIL
bool is_inactive_itemset(itemset_t itemset)
{
	return (itemset.shape == NIL);
}

// Returns true if two given itemsets could be joined to make new candidate itemset, otherwise false.
// E.g. itemsets (1,2,3) and (1,2,4) are joinable into (1,2,3,4) candidate itemset 
// but itemsets (1,2,3) and (1,5,6) or (1,2,3) and (4,5,6) are not
// --------------
// In fact this function returns true iff a and b differ only in one bit.
// This found by counting 1 in c=(a xor b), i.e. number of 1 in c should be 2.
bool joinable(bitmask_t a, bitmask_t b)
{
	return (POPCNT(a ^ b) == 2);
#ifdef __NEVER
	bitmask_t count = 0;

	for (bitmask_t c = a ^ b; c != 0; c = c & (c - 1)) {
		count++;
		if (count > 2)
			return false;
	}
	return (count == 2);
#endif
}

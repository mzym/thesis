/*
Parallel Dynamic Itemset Counting.
Definitions of data structures and interface of supplemental methods.

(c) 2016 Mikhail Zymbler
*/

#ifndef ITEMSETS_H
#define ITEMSETS_H

#include "bitmap.h"
#include <vector>

// An item
typedef unsigned int item_t;

// Shape of an itemset
typedef char shape_t;

// Possible shapes of an itemset
#define BOX ((shape_t) 0)
#define CIRCLE ((shape_t) 1)
#define NIL ((shape_t) -1) // NIL-shaped itemset should be deleted from a set.

// An itemset
typedef struct itemset_t {
	itemset_bitmask_t bitmask;	// Bitmask of an itemset is an array of unsigned long long integers where bits in elements are set iff the itemset contains the respective item.
	unsigned long k;			// Number of items in itemset (i.e. number of bits in bitmask that set to 1)
	unsigned long stopno;		// Current stop
	unsigned long long support;	// Support count
	shape_t shape;				// Shape
	bool operator==(const itemset_t& other) const
	{
		//return (bitmask == other.bitmask);
		for (unsigned int i = 0; i < TRANSACTION_BITMASK_LEN; i++)
			if (bitmask[i] != other.bitmask[i])
				return false;
		return true;
	}
} itemset;

// Returns true if an itemset's shape is NIL
// (this function needed to perform remove inactive itemsets from sets via STL's remove_if)
bool is_inactive_itemset(itemset_t itemset);

// Returns true if two given itemsets could be joined to make new candidate itemset, otherwise false.
// E.g. itemsets (1,2,3) and (1,2,4) are joinable into (1,2,3,4) candidate itemset 
// but itemsets (1,2,3) and (1,5,6) or (1,2,3) and (4,5,6) are not
// --------------
// In fact this function returns true iff a and b differ only in one bit.
// This found by counting 1 in c=(a xor b), i.e. number of 1 in c should be 2.
bool joinable(bitmask_t a, bitmask_t b);

#endif
/*
Parallel Dynamic Itemset Counting.
Interface of data reader that reads bitmap of transaction database.

(c) 2016 Mikhail Zymbler
*/

#ifndef BITMAP_H
#define BITMAP_H

#include "params.h"

// Bit mask
#ifdef _WIN64
typedef unsigned long long int bitmask_t;
#else
typedef unsigned __int64 bitmask_t;
#endif

// Number of bitmasks in transaction's or itemset's bitmap
#define BITMASK_LEN_MAX	(m_MAX/64 + 1)

// Transaction bitmask
typedef bitmask_t transaction_bitmask_t[BITMASK_LEN_MAX];

// Itemset bitmask
typedef transaction_bitmask_t itemset_bitmask_t;

// Bitmap of transaction database.
// Each transaction in database is represented by its bitmask.
// Bitmask of a transaction is an array of unsigned long long integers where bits in elements are set iff the transaction contains the respective item.
typedef bitmask_t **bitmap_t;

extern bitmap_t BITMAP; // Bitmap of the transaction database 
extern unsigned int TRANSACTION_BITMASK_LEN; // Number of bitmasks in the bitmap

// Initialization of (allocation memory for) bitmap.
// Input:
// n -- number of transactions
// m -- number of items
// Output: none.
// Side effect: 
// Creates BITMAP object in RAM.
// Returns 0 if success or negative value in case of error.
int init_bitmap(unsigned long long n, unsigned int m);

// Destroying of bitmap.
// Input: 
// n -- number of transactions.
// m -- number of items
// Side effect: 
// Deletes BITMAP object from RAM.
// Returns 0 if success or negative value in case of error.
int destroy_bitmap(unsigned long long n, unsigned int m);

// Bit tricks
#define setbit(byte, bit)	((byte) |= ((bitmask_t) 1) << (bit))
#define clrbit(byte, bit)	((byte) &= ~(((bitmask_t) 1) << (bit)))
#define isbitset(byte, bit)	(((byte) & (((bitmask_t) 1) << (bit))) != 0)
#define isbitclr(byte, bit)	(((byte) & (((bitmask_t) 1) << (bit))) == 0)

// Returns true if two given itemsets could be joined to make new candidate itemset, otherwise false.
// E.g. itemsets (1,2,3) and (1,2,4) are joinable into (1,2,3,4) candidate itemset 
// but itemsets (1,2,3) and (1,5,6), (1,2,3) and (4,5,6) are not
bool joinable(bitmask_t a, bitmask_t b);

// Read bit map of transaction database from a file.
// Input:
// inpname -- name of binary file to read (i.e. a file where each row represents a bitmap of a transaction)
// Output:
// n -- resulting number of transactions
// m -- resulting number of items
// Side effect: 
// Creates BITMAP object in RAM.
// Returns 0 if success or negative value in case of error.
int read_bitmap(char * inpname, unsigned long long &n, unsigned int &m);

// Make bitmap.
// Input:
// inpname -- name of text file to read (i.e. a file where each row represents a transaction where items are separated by a delimiter).
//			Note that A) numbering of items must start from 1 B) row must end with EOLN, not space.
// delimiter -- string to separate values in the inpfile
// outname -- name of binary file to write tid-bitmap.
// Output:
// n -- number of transactions
// m -- number of items
// Side effects: 
// 1) Creates a binary file outname with bitmap. First two ULL numbers are n (number of transactions) and m (number of items). The rest ULLs are rows of bitmap.
// 2) Creates BITMAP object in RAM.
// Returns 0 if success or negative value in case of error.
int make_bitmap(char * inpname, char * delimiter, char * outname, unsigned long long int &n, unsigned int &m);

// Make transaction database without zeros.
// Input:
// inpname -- name of text file to read (i.e. a file where each row represents a transaction where items are separated by a delimiter).
//			Note that row must end with EOLN, not space.
// delimiter -- string to separate values in the inpfile
// outname -- name of file to write database where each item increased by 1.
// Side effects: 
// Creates a text file with trabsaction database where each item increased by 1.
// Returns 0 if success or negative value in case of error.
int subzero(char * inpname, char * delimiter, char * outname);

// Make transaction database without zeros.
// Input:
// inpname -- name of text file to read (i.e. a file where each row represents a transaction where items are separated by a delimiter).
//			Note that row must end with EOLN, not space.
// delimiter -- string to separate values in the inpfile
// outname -- name of file to write database where each item increased by 1.
// Side effects: 
// Creates a text file with trabsaction database where each item increased by 1.
// Returns 0 if success or negative value in case of error.
int subzero_rmendblanks(char * inpname, char * delimiter, char * outname);

#endif
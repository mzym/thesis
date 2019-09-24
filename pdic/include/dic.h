/*
Parallel Dynamic Itemset Counting.
Interface of the main method of the algorithm.

(c) 2016 Mikhail Zymbler
*/

#ifndef DIC_H
#define DIC_H

// Statistics on algorithm's execution
typedef struct dic_stat_t {
	double passes;					// Òumber of passes over transaction database
	unsigned long long candtotal;	// Total number of candidates generated during execution
	unsigned long long candpruned;	// Number of candidates pruned
} dic_stat;

extern dic_stat_t DICSTAT;		// Statistics on run

double 							// Returns: runtime of the algorithm
dic(							// Input:
								// Bitmap of transaction database (global object BITMAP defined in "bitmap.h")
	unsigned long long int n,	// number of transactions
	unsigned int m,				// number of items
	unsigned long long minsup,	// minimum support count
	unsigned long long M,		// number of transactions to be processed before the stop
	int num_of_threads);		// number of threads to run the algorithm
								// Output: 
								// statistics on execution				
								// frequent itemsets (global object BOX defined in "itemsets.h")

// Print SOLID BOXes
// Input: n -- number of transactions
void print_solid(unsigned long long int n);

#endif
/*
Parallel Dynamic Itemset Counting.
Making bitmap.

(c) 2017 Mikhail Zymbler
*/

#include "debugger.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Parameters of execution
char * inp_fname;			// filename to read transaction database
char * delimiter;			// delimiter of items in transaction
char * bitmap_fname;		// filename to write bitmap of transaction database
unsigned long long n;		// number of transactions
unsigned int m;				// number of items


int main(int argc, char * argv[])
{
	if (argc < 3) {
		printf("Make bitmap of transaction database. Usage with the following parameters:\n");
		printf("No.\tSemantic\n");
		printf("1\tfilename to read txt file of transaction database\n");
		printf("2\tdelimiter of itemsets in txt file of transaction database\n");
		printf("3\tfilename to write bitmap\n");
		return 0;
	}

	// Get command line parameters
	inp_fname = argv[1];
	assert(inp_fname != NULL);
	delimiter = argv[2];
	assert(delimiter != NULL);
	bitmap_fname = argv[3];
	assert(bitmap_fname != NULL);

	int res = make_bitmap(inp_fname, delimiter, bitmap_fname, n, m);
	assert(res == 0);

//	PRINT_BITMAP("Bitmap was created:");

	res = destroy_bitmap(n, m);
	assert(res == 0);

	return 0;
}


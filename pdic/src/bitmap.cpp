/*
Parallel Dynamic Itemset Counting.
Implementation of data reader that reads bitmap of transaction database.

(c) 2016 Mikhail Zymbler
*/

#include "bitmap.h"
#include "itemsets.h"
#include "system.h"
#include "debugger.h"
#include "csvparser.h"
#include <assert.h>
#include <stdio.h>


bitmap_t BITMAP; // Bitmap of the transaction database
unsigned int TRANSACTION_BITMASK_LEN; // Number of bitmasks in the bitmap

// Initialization of (allocation memory for) bitmap.
// Input:
// n -- number of transactions
// m -- number of items
// Output: none.
// Side effect: 
// Creates BITMAP object in RAM.
// Returns 0 if success or negative value in case of error.
int init_bitmap(unsigned long long n, unsigned int m)
{
	BITMAP = (bitmask_t **)_align_malloc(n * sizeof(bitmask_t *));
	assert(BITMAP != NULL);
	for (unsigned int i = 0; i < n; i++) {
		BITMAP[i] = (bitmask_t *)_align_malloc(TRANSACTION_BITMASK_LEN * sizeof(bitmask_t));
		assert(BITMAP[i] != NULL);
	}

	for (unsigned int i = 0; i < n; i++)
		for (unsigned long long int j = 0; j < TRANSACTION_BITMASK_LEN; j++)
			BITMAP[i][j] = 0;

	return 0;
}

// Destroying of bitmap.
// Input: 
// n -- number of transactions.
// m -- number of items
// Side effect: 
// Deletes BITMAP object from RAM.
// Returns 0 if success or negative value in case of error.
int destroy_bitmap(unsigned long long n, unsigned int m)
{
	for (unsigned int i = 0; i < n; i++) {
		_align_free(BITMAP[i]);
	}
	_align_free(BITMAP);
	return 0;
}

// Read bit map of transaction database from a file.
// Input:
// inpname -- name of binary file to read (i.e. a file where each row represents a bitmap of a transaction)
// Output:
// n -- resulting number of transactions
// m -- resulting number of items
// Side effect: 
// Creates BITMAP object in RAM.
// Returns 0 if success or negative value in case of error.
int read_bitmap(char * inpname, unsigned long long &n, unsigned int &m)
{
	START("Read bitmap");

	FILE * f = fopen(inpname, "rb");
	PRINT("File==`%s'\t", inpname);
	assert(f != NULL);
	fread(&(n), sizeof(unsigned long long int), 1, f);
	PRINT("n==%llu\t", n);
	assert(n >= 1 && n <= n_MAX);
	fread(&(m), sizeof(unsigned int), 1, f);
	PRINT("m==%lu\t", m);
	assert(m >= 1 && m <= m_MAX);
	fread(&TRANSACTION_BITMASK_LEN, sizeof(unsigned int), 1, f);
	assert(TRANSACTION_BITMASK_LEN >= 1 && TRANSACTION_BITMASK_LEN <= BITMASK_LEN_MAX);

	PRINT("Number of transactions n==%llu\nNumber of items m==%d\nBitmask len==%d\n", n, m, TRANSACTION_BITMASK_LEN);

	int res = init_bitmap(n, m);
	assert(res == 0);

	for (unsigned int i = 0; i < n; i++) {
		for (unsigned long long j = 0; j < TRANSACTION_BITMASK_LEN; j++) {
			fread(&BITMAP[i][j], sizeof(bitmask_t), 1, f);
			assert(BITMAP[i][j] >= 0);
			//			PRINT("TIDBITMAP[%lu][%llu]==%llu\t", i, j, TIDBITMAP[i][j]);
		}
	}
	fclose(f);
	FINISH("Read bitmap");

	return 0;
}

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
int make_bitmap(char * inpname, char * delimiter, char * outname, unsigned long long int &n, unsigned int &m)
{
	START("make_bitmap");
	// Open input file to determine the number of transactions and the number of items.
	CsvParser *csvparser = CsvParser_new(inpname, delimiter, 0);
	assert(csvparser != NULL);
	CsvRow *row;
	n = 0;
	m = 0;
	while ((row = CsvParser_getRow(csvparser))) {
		const char **rowFields = CsvParser_getFields(row);
		unsigned int N = CsvParser_getNumFields(row);
		assert(N >= 0);
		for (unsigned int i = 0; i < N; i++) {
			PRINT("FIELD: '%s'\t", rowFields[i]);
			item_t item = atoi(rowFields[i]);
			PRINT("item==%lu\n", item);
			assert(item >= 1 && item <= m_MAX);
			if (m < item) {
				m = item;
			}
		}
		CsvParser_destroy_row(row);
		n++;
	}
	CsvParser_destroy(csvparser);

	// Create output file and write there the number of transactions and the number of items.
	assert(n >= 1 && n <= n_MAX);
	assert(m >= 1 && m <= m_MAX);
	TRANSACTION_BITMASK_LEN = m / 64 + 1;
	assert(TRANSACTION_BITMASK_LEN >= 1 && TRANSACTION_BITMASK_LEN <= BITMASK_LEN_MAX);

	FILE * outfile = fopen(outname, "wb");
	assert(outfile != NULL);
	//	fprintf(outfile, "%llu\n%d\n", *n, *m);
	fwrite(&n, sizeof(unsigned long long int), 1, outfile);
	fwrite(&m, sizeof(unsigned int), 1, outfile);
	fwrite(&TRANSACTION_BITMASK_LEN, sizeof(unsigned int), 1, outfile);

	int res = init_bitmap(n, m);
	assert(res == 0);

	// Open input file again, process it and fill in TID bitmap.
	csvparser = CsvParser_new(inpname, delimiter, 0);
	assert(csvparser != NULL);
	unsigned long long int t = 0;
	while ((row = CsvParser_getRow(csvparser))) {
		const char **rowFields = CsvParser_getFields(row);
		unsigned int N = CsvParser_getNumFields(row);
		for (unsigned int i = 0; i < N; i++) {
			//PRINT("TRANSACTION #%llu\tFIELD: '%s'\t", t + 1, rowFields[i]);
			item_t item = atoi(rowFields[i]);
			assert(item >= 1 && item <= m_MAX);
			//PRINT("TRANSACTION #%llu\tITEM==%lu\n", t + 1, item);
			if (item >= 1 && item <= m_MAX) {
				//PRINT("BITMAP[%llu][%d]==%llu\n", t, (item - 1) / 64, BITMAP[t][(item - 1) / 64]);
				setbit(BITMAP[t][(item - 1) / 64], (item - 1) % 64);
				PRINT("BITMAP[%llu][%d]==%llu\n", t, (item - 1) / 64, BITMAP[t][(item - 1) / 64]);
			}
		}
		CsvParser_destroy_row(row);
		t++;
	}
	CsvParser_destroy(csvparser);

	PRINT_BITMAP("Bitmap is created:");

	// Write tid-bitmap to the output file.
	for (unsigned long long int t = 0; t < n; t++) {
		for (unsigned int i = 0; i < TRANSACTION_BITMASK_LEN; i++) {
			PRINT("Written BITMAP[%llu][%lu]==%llu\n", t, i, BITMAP[t][i]);
			fwrite(&(BITMAP[t][i]), sizeof(bitmask_t), 1, outfile);
		}
	}

	fclose(outfile);
	FINISH("make_bitmap");

	return 0;
}

// Make transaction database without zeros.
// Input:
// inpname -- name of text file to read (i.e. a file where each row represents a transaction where items are separated by a delimiter).
//			Note that row must end with EOLN, not space.
// delimiter -- string to separate values in the inpfile
// outname -- name of file to write database where each item increased by 1.
// Side effects: 
// Creates a text file with trabsaction database where each item increased by 1.
// Returns 0 if success or negative value in case of error.
int subzero(char * inpname, char * delimiter, char * outname)
{
	START("subzero");
	CsvParser *csvparser = CsvParser_new(inpname, delimiter, 0);
	assert(csvparser != NULL);
	FILE * outfile = fopen(outname, "wt");
	assert(outfile != NULL);
	CsvRow *row;
	while ((row = CsvParser_getRow(csvparser))) {
		const char **rowFields = CsvParser_getFields(row);
		unsigned int N = CsvParser_getNumFields(row);
		assert(N > 0);
		for (unsigned int i = 0; i < N; i++) {
			item_t item = atoi(rowFields[i]);
			assert(item >=0 && item <= m_MAX);
			fprintf(outfile, "%lu", item + 1);
			if (i != N - 1)
				fprintf(outfile, " ");
		}
		CsvParser_destroy_row(row);
		fprintf(outfile, "\n");
	}
	CsvParser_destroy(csvparser);
	fclose(outfile);
	FINISH("subzero");

	return 0;
}

// Make transaction database without zeros.
// Input:
// inpname -- name of text file to read (i.e. a file where each row represents a transaction where items are separated by a delimiter).
//			Note that row must end with EOLN, not space.
// delimiter -- string to separate values in the inpfile
// outname -- name of file to write database where each item increased by 1.
// Side effects: 
// Creates a text file with trabsaction database where each item increased by 1.
// Returns 0 if success or negative value in case of error.
int subzero_rmendblanks(char * inpname, char * delimiter, char * outname)
{
	START("subzero_rmendblanks");
	CsvParser *csvparser = CsvParser_new(inpname, delimiter, 0);
	assert(csvparser != NULL);
	FILE * outfile = fopen(outname, "wt");
	assert(outfile != NULL);
	CsvRow *row;
	while ((row = CsvParser_getRow(csvparser))) {
		const char **rowFields = CsvParser_getFields(row);
		unsigned int N = CsvParser_getNumFields(row);
		assert(N > 0);
		for (unsigned int i = 0; i < N; i++) {
			item_t item = atoi(rowFields[i]);
			assert(item >= 0 && item <= m_MAX);
			if (i == 0)
				fprintf(outfile, "%lu", item + 1);
			else
				if (i == N - 1) {
					if (item > 0)
						fprintf(outfile, " %lu", item + 1);
				}
				else
					fprintf(outfile, " %lu", item + 1);
		}
		CsvParser_destroy_row(row);
		fprintf(outfile, "\n");
	}
	CsvParser_destroy(csvparser);
	fclose(outfile);
	FINISH("subzero_rmendblanks");

	return 0;
}
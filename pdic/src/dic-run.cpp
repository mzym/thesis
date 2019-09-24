/*
Parallel Dynamic Itemset Counting.
Executor of the main method for testing.

(c) 2017 Mikhail Zymbler
*/

#include "params.h"
#include "debugger.h"
#include "profiler.h"
#include "system.h"
#include "itemsets.h"
#include "omp.h"
#include "dic.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifndef _WIN64
#include "immintrin.h"
#endif

#define NMAKE_BITMAP
#define REPORT
#define PROFILE

// Parameters of execution
char * bitmap_fname;		// = "C:\\Users\\Михаил\\Documents\\Visual Studio 2015\\Projects\\FIM\\paral-dic\\x64\\Debug\\pujari.dat"; // filename to read bitmap of transaction database
unsigned long long n;		// number of transactions
unsigned int m;				// number of items
unsigned long long minsup;	// minimum support count
unsigned long long M;		// number of transactions to be processed - before the stop
unsigned int num_of_threads;// number of threads to run the algorithm

double runtime;

int main(int argc, char * argv[])
{
	if (argc < 4) {
		printf("Parallel Dynamic Itemset Counting. Usage with the following parameters:\n");
		printf("No.\tSemantic\n");
		printf("1\tfilename to read bitmap of transaction database\n");
		printf("2\tminsup, minimum support (percentage, 0..1)\n");
		printf("3\tM, number of transactions to be processed before stop\n");
		printf("4\tT, number of threads to run the algorithm, from 1 to %u (%u by default)\n", omp_get_max_threads(), omp_get_max_threads());
		return 0;
	}

	// Get command line parameters
	bitmap_fname = argv[1];
	assert(bitmap_fname != NULL);
	double threshold = atof(argv[2]);
	assert(threshold > 0 && threshold < 1);
	M = atoll(argv[3]);
	num_of_threads = (argc == 5 ? atoi(argv[4]) : omp_get_max_threads());
	assert(num_of_threads >= 1 && num_of_threads <= (unsigned int)omp_get_max_threads());

	int res = read_bitmap(bitmap_fname, n, m);
	assert(res == 0);
	
	printf("Dataset: %s\n\tn==%llu\n\tm==%llu\n", bitmap_fname, n, m);

	// For the sake of simplicity and without loss of generality make n divisible by M
	while (n % M != 0) n--;

	assert(n >= 1 && n <= n_MAX);
	assert(m >= 1 && m <= m_MAX);
	minsup = (unsigned long long int)(n * threshold);
	assert(minsup >= 1 && minsup <= n);
	assert(M >= 1 && M <= n / 2);

	runtime = dic(n, m, minsup, M, num_of_threads);
	printf("%.4f\n", runtime);

#ifdef REPORT
	printf("Dataset: %s\nn\tm\tminsup\tM\tT\ttime\tpasses\tcandidates\tpruned\n", bitmap_fname);
	printf("%llu\t%u\t%.2f\t%llu\t%u\t%.4f\t%.2f\t%llu\t%llu (%.2f)\n", n, m, ((double)minsup) / n, M, num_of_threads, runtime, DICSTAT.passes, DICSTAT.candtotal, DICSTAT.candpruned, ((double)DICSTAT.candpruned)/DICSTAT.candtotal);

	print_solid(n);
#endif
#ifndef NPROFILE
	printf("1\t%.2f%%\n2\t%.2f%%\n3\t%.2f%%\n4\t%.2f%%\n5\t%.2f%%\n6\t%.2f%%\n7\t%.2f%%\n8\t%.2f%%\n9\t%.2f%%\n10\t%.2f%%\n", \
		(finish1 - start1) / (runtime) * 100, \
		(finish2 - start2) / (runtime) * 100, \
		(finish3 - start3) / (runtime) * 100, \
		(finish4 - start4) / (runtime) * 100, \
		(finish5 - start5) / (runtime) * 100, \
		(finish6 - start6) / (runtime) * 100, \
		(finish7 - start7) / (runtime) * 100, \
		(finish8 - start8) / (runtime) * 100, \
		(finish9 - start9) / (runtime) * 100, \
		(finish10 - start10) / (runtime) * 100);
#endif

	return 0;
}


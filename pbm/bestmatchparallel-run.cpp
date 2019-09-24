/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Exector.
(c) Mikhail Zymbler 
*/


#include "params.h"
#include "debugger.h"
#include "system.h"
#include "reader.h"
#include "bestmatchparallel.h"
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <cmath>
#include <malloc.h>

using namespace std;

// Parameters of execution
char* timeseries_fname; 	 // filename to read time series
char* query_fname;		 // filename to read query
int r;				 // size of Sakoe-chiba warping band
int lenquery;			 // length of subsequence	
int num_of_threads;		 // number of threads to run the algorithm
int k;

int main(int argc, char* argv[])
{
	int rank, size, provided;
	double R;
	
	MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
	
	MPI_Comm_size(MPI_COMM_WORLD, &size); //count of processes
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); //number of process

	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	printf("PID %d on %s ready for attach\n", getpid(), hostname);

	timeseries_fname = argv[1];
	assert(timeseries_fname != NULL);
	query_fname = argv[2];
	assert(timeseries_fname != NULL);
	lenquery = atoi(argv[3]);
	R = atof(argv[4]);
	if (R <= 1)
		r = (int)floor(R*lenquery);
    	else
        	r = (int)floor(R);	 	
	k = atoi(argv[5]);
	num_of_threads = atoi(argv[6]);
	assert(num_of_threads >= 1 && num_of_threads <= omp_get_max_threads());	
	
	omp_set_num_threads(num_of_threads);

	read_query(query_fname, lenquery, rank);

	read_timeseries(timeseries_fname, lenquery, size, rank);
	
	bestmatch(lenquery, num_of_threads, r, k, rank, size);

	MPI_Finalize();
	
	return 0;
}

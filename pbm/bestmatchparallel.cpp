/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
(c) Mikhail Zymbler 
*/


#include "bestmatchparallel.h"
#include "system.h"
#include "params.h"
#include "dtw.h"
#include "lb.h"
#include "reader.h"
#include "norm.h"
#include "envelope.h"
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>
#include <iostream>
#include "profiler.h"


using namespace std;

#define min(x,y) ((x)<(y)?(x):(y))
#define PROFILE

struct Subsequence 
{
	float dist;
	int position; //position of subsequence in time series
};

void bestmatch(int lenquery, int num_of_threads, int r, int k, int rank, int size) {

	float bsf = FLT_MAX, bsf_min = FLT_MAX, local_bsf = 0;
	int finished = (int)false, allfinished = (int)false;
	bool full_DTW_matrix = false;
	int current_size_DTW_matrix = 0, num_of_segments = num_of_threads, handleSubseq = 0, result_position = 0, current_segment = 0, countSubsInSegment = 0, num_calc_DTW = 0;
	
	int max_CountSubsInFragment = 0;
	MPI_Allreduce(&CountSubsInFragment, &max_CountSubsInFragment, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	Subsequence total_result_subsequence, local_result_subsequence;
	total_result_subsequence.dist = FLT_MAX;
	local_result_subsequence.dist = FLT_MAX;

	init_matrix(lenquery);
	float* lb1 = (float*)_align_malloc(CountSubsInFragment * sizeof(float)); //array for LB_Kim
	float* lb2 = (float*)_align_malloc(CountSubsInFragment * sizeof(float)); //array for LB_Keogh
	float* lb3 = (float*)_align_malloc(CountSubsInFragment * sizeof(float)); //array for LB_Keogh_EC
    	int* bitmap_lb1 = (int*)_align_malloc(CountSubsInFragment * sizeof(int)); //bitmap for LB_Kim
	int* bitmap_lb2 = (int*)_align_malloc(CountSubsInFragment * sizeof(int)); //bitmap for LB_Keogh
	int* bitmap_lb3 = (int*)_align_malloc(CountSubsInFragment * sizeof(int)); //bitmap for LB_Keogh_EC
	int* result_bitmap = (int*)_align_malloc(CountSubsInFragment * sizeof(int));

	int* endSegment = (int*)_align_malloc(num_of_threads * sizeof(int));
	int* positionInSegment = (int*)_align_malloc(num_of_threads * sizeof(int));
	int* position = (int*)_align_malloc(k * num_of_threads * sizeof(int));
	
	
	float envelope_matrix[128][205] __attribute__((aligned(64)));
	
	PRF_INIT;

	double start = MPI_Wtime();
	normalize_query(lenquery);
	envelope_query(lenquery, r, envelope_matrix);


//start1
PRF_START(start1)
#pragma omp parallel for num_threads(num_of_threads)
	for (int i = 0; i < CountSubsInFragment; i++) {
		normalize(lenquery, i);
		lb1[i] = LB_Kim(lenquery, i);
		lb2[i] = LB_Keogh(lenquery, i);
		envelope_subsequence(lenquery, r, i);
		lb3[i] = LB_Keogh_EC(lenquery, i);
	}
PRF_FINISH(finish1); 
//end1


PRF_START(start4);
#pragma omp parallel for num_threads(num_of_threads)
	for (int i = 0; i < CountSubsInFragment; i++) {
#pragma vector aligned	
#pragma ivdep
		for (int j = 0; j < lenquery; j++) {
		    UPPER[i][j] = (lb2[i] > lb3[i]) ? UPPER_q[j] : UPPER[i][j];
		    LOWER[i][j] = (lb2[i] > lb3[i]) ? LOWER_q[j] : LOWER[i][j];
		}
	}
PRF_FINISH(finish4);


	bsf = dtw(0, r, lenquery, bsf, lb2[0] < lb3[0]);
	

	local_result_subsequence.position = rank*CountSubsInFragment;
        local_result_subsequence.dist = bsf;
		
	MPI_Allreduce(&local_result_subsequence, &total_result_subsequence, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
	bsf = total_result_subsequence.dist;

	countSubsInSegment = (int)ceilf((float)CountSubsInFragment / num_of_segments); // length of segment

//start2
PRF_START(start2);
#pragma omp parallel for num_threads(num_of_threads)
	for (int i = 0; i < num_of_segments; i++) {
		positionInSegment[i] = i * countSubsInSegment;
		endSegment[i] = (i + 1) * countSubsInSegment;
	}	
	endSegment[num_of_segments - 1] = CountSubsInFragment;
PRF_FINISH(finish2);
//end2


//start3
PRF_START(start3);
	while (!allfinished) {
		full_DTW_matrix = false;

#pragma omp parallel num_threads(num_of_threads)
	{	
		__ASSUME_ALIGNED(bitmap_lb1, ALIGN_SIZE);
		__ASSUME_ALIGNED(bitmap_lb2, ALIGN_SIZE);
		__ASSUME_ALIGNED(bitmap_lb3, ALIGN_SIZE);
		__ASSUME_ALIGNED(lb1, ALIGN_SIZE);
		__ASSUME_ALIGNED(lb2, ALIGN_SIZE);
		__ASSUME_ALIGNED(lb3, ALIGN_SIZE);
		__ASSUME_ALIGNED(result_bitmap, ALIGN_SIZE);
		int current_thread = omp_get_thread_num();
		for (int i = positionInSegment[current_thread]; i < endSegment[current_thread]; i++) {
			bitmap_lb1[i] = (lb1[i] < bsf);
			bitmap_lb2[i] = (lb2[i] < bsf);
			bitmap_lb3[i] = (lb3[i] < bsf);
			result_bitmap[i] = bitmap_lb1[i] && bitmap_lb2[i] && bitmap_lb3[i];
		}
	}

		current_size_DTW_matrix = 0;
		while (!full_DTW_matrix) {
			if (positionInSegment[current_segment] < endSegment[current_segment]) {
				if (result_bitmap[positionInSegment[current_segment]]) {
					current_size_DTW_matrix++;
					position[current_size_DTW_matrix - 1] = positionInSegment[current_segment];
				}

				positionInSegment[current_segment]++;
				handleSubseq++;
			}

			current_segment++;
			if (current_segment == num_of_segments) 
				current_segment = 0;

			if (current_size_DTW_matrix == num_of_threads * k)
				break;
			if (handleSubseq == CountSubsInFragment) {
				finished = 1;
				break;
			}
		}

#pragma omp parallel for  shared(bsf, result_position) private(local_bsf) num_threads(num_of_threads)
		for (int i = 0; i < current_size_DTW_matrix; i++) {
			local_bsf = dtw(position[i], r, lenquery, bsf, lb2[position[i]] < lb3[position[i]]);	
			#pragma omp critical 
			{
				if (bsf > local_bsf) {
					bsf = local_bsf;	 
					result_position = position[i];	
					local_result_subsequence.position = result_position+rank*max_CountSubsInFragment;
                			local_result_subsequence.dist = bsf;
				}
			}
		}
		num_calc_DTW += current_size_DTW_matrix;
	
		MPI_Allreduce(&finished, &allfinished, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
		MPI_Allreduce(&local_result_subsequence, &total_result_subsequence, 1, MPI_FLOAT_INT, MPI_MINLOC, MPI_COMM_WORLD);
				
		bsf = total_result_subsequence.dist;
		if (rank == 0)
			printf("bsf = %f position = %d allfinished = %d\n", total_result_subsequence.dist, total_result_subsequence.position, allfinished);
	}
PRF_FINISH(finish3);
//end3	
	

	if (rank == 0) {
		printf("bsf = %.5lf\n", sqrt(total_result_subsequence.dist));
		printf("position = %d\n", total_result_subsequence.position);
		printf("Total Runtime of finding the best subsequence %.2lf\n", MPI_Wtime()-start);
	}

}

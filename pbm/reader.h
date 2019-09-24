/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Data reader.
(c) Mikhail Zymbler 
*/

#ifndef READER_H
#define READER_H

#include "params.h"

void read_timeseries(char * filename, int lenquery, int size, int rank);

void read_query(char* filename, int lenquery, int rank);
		
extern float* QUERY;
extern float** SUBSMATRIX;
extern int CountSubsInFragment;

#endif

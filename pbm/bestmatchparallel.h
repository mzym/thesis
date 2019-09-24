/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
(c) Mikhail Zymbler 
*/

#ifndef BESTMATCH_H
#define BESTMATCH_H

#include "reader.h"
#include "params.h"

void bestmatch(int lenquery, int num_of_threads, int r, int k, int rank, int size);	

#endif

/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Z-normalization of subsequence.
(c) Mikhail Zymbler 
*/

#ifndef NORM_H
#define NORM_H

#include "reader.h"
#include "params.h"

void normalize_query(int lenquery);

void normalize(int lenquery, int t);

#endif

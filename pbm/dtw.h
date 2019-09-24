/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of DTW measure.
(c) Mikhail Zymbler 
*/

#ifndef DTW_H
#define DTW_H

#include "reader.h"
#include "envelope.h"
#include "params.h"

float dtw(int t, int r, int lenquery, float bsf, bool big_lb);

#endif

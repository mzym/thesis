/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of envelopes for LB_Keogh and LB_Keogh_EC lower bounds.
(c) Mikhail Zymbler 
*/

#ifndef ENVELOP_H
#define ENVELOP_H

#include "reader.h"
#include "params.h"

void envelope_query(int lenquery, int r, float matrix[128][205]);

void envelope_subsequence(int lenquery, int r, int t);

void init_matrix(int lenquery);

extern float** UPPER;
extern float** LOWER;
extern float* LOWER_q;
extern float* UPPER_q;
#endif

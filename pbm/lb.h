/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of lower bounds (LB_Kim, LB_Keogh, LB_Keogh_EC).
(c) Mikhail Zymbler 
*/


#ifndef LB_H
#define LB_H

#include "reader.h"
#include "envelope.h"
#include "params.h"

float LB_Kim(int lenquery, int i);

float LB_Keogh(int lenquery, int t);

float LB_Keogh_EC(int lenquery, int t);

#endif

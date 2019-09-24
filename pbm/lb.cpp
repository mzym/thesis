/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of lower bounds (LB_Kim, LB_Keogh, LB_Keogh_EC).
(c) Mikhail Zymbler 
*/

#include "lb.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
#include "system.h"

#define dist(x,y) ((x-y)*(x-y))
#define min(x,y) ((x)<(y)?(x):(y))

float LB_Kim(int lenquery, int i)
{
	float lb_Kim = 0, d = 0;

	//1 point at front and back
	lb_Kim = dist(QUERY[0], SUBSMATRIX[i][0]) + dist(QUERY[lenquery-1], SUBSMATRIX[i][lenquery-1]);
	
	//2 points at front
	d = min(dist(QUERY[1], SUBSMATRIX[i][0]), dist(QUERY[0], SUBSMATRIX[i][1]));
	d = min(d, dist(QUERY[1], SUBSMATRIX[i][1]));
	lb_Kim += d;

	//2 points at back
	d = min(dist(QUERY[lenquery-2], SUBSMATRIX[i][lenquery-1]), dist(QUERY[lenquery-1], SUBSMATRIX[i][lenquery-2]));
	d = min(d, dist(QUERY[lenquery-2], SUBSMATRIX[i][lenquery-2]));
	lb_Kim += d;

	//3 points at front
	d = min(dist(QUERY[2], SUBSMATRIX[i][0]), dist(QUERY[2], SUBSMATRIX[i][1]));
	d = min(d, dist(QUERY[2], SUBSMATRIX[i][2]));
	d = min(d, dist(QUERY[1], SUBSMATRIX[i][2]));
	d = min(d, dist(QUERY[0], SUBSMATRIX[i][2]));
	lb_Kim += d;

	//3 points at back
	d = min(dist(QUERY[lenquery-3], SUBSMATRIX[i][lenquery-1]), dist(QUERY[lenquery-3], SUBSMATRIX[i][lenquery-2]));
	d = min(d, dist(QUERY[lenquery-3], SUBSMATRIX[i][lenquery-3]));
	d = min(d, dist(QUERY[lenquery-2], SUBSMATRIX[i][lenquery-3]));	
	d = min(d, dist(QUERY[lenquery-1], SUBSMATRIX[i][lenquery-3]));
	lb_Kim += d;

	return lb_Kim;
}

float LB_Keogh(int lenquery, int t)	// Returns: LB_Keogh
{
	float d = 0, lb_keogh = 0;

    #pragma vector aligned
	for (int k = 0; k < lenquery; k++) {
		d = (SUBSMATRIX[t][k] > UPPER_q[k]) ? dist(SUBSMATRIX[t][k], UPPER_q[k]) : dist(SUBSMATRIX[t][k], LOWER_q[k]);
		d = ((SUBSMATRIX[t][k] >= LOWER_q[k]) && (SUBSMATRIX[t][k] <= UPPER_q[k])) ? 0 : d;
		lb_keogh += d;
	}

	return lb_keogh;
}


float LB_Keogh_EC(int lenquery, int t)	// Returns: LB_Keogh, LB_KeoghEC
{
	float d = 0, lb_keogh = 0;

    #pragma vector aligned 
	for (int k = 0; k < lenquery; k++) {
		d = (QUERY[k] > UPPER[t][k]) ? dist(QUERY[k], UPPER[t][k]) : dist(QUERY[k], LOWER[t][k]);
		d = ((QUERY[k] >= LOWER[t][k]) && (QUERY[k] <= UPPER[t][k])) ? 0 : d;
		lb_keogh += d;
	}

	return lb_keogh;
}

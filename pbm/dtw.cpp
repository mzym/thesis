/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of DTW measure.
(c) Mikhail Zymbler 
*/

#include "dtw.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include "system.h"
#include <malloc.h>
#include <omp.h>
#include <string.h>

#define dist(x,y) ((x-y)*(x-y))
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))


float dtw(int t, int r, int lenquery, float bsf, bool big_lb)	
{

	float* cb = (float*)_align_malloc(lenquery * sizeof(float));
	float d = 0;

	if (big_lb) {
	    	__ASSUME_ALIGNED(QUERY, ALIGN_SIZE);
		for (int w = 0; w < lenquery; w++) {
			cb[w] = QUERY[w];
		}
	}
	else
#pragma vector aligned	
	    for (int w = 0; w < lenquery; w++)
		cb[w] = SUBSMATRIX[t][w];

		
	for (int p = lenquery - 1; p >= 0; p--) {
		d = (cb[p] > UPPER[t][p]) ? dist(cb[p], UPPER[t][p]) : dist(cb[p], LOWER[t][p]);
		d = ((cb[p] >= LOWER[t][p]) && (cb[p] <= UPPER[t][p])) ? 0 : d;

		cb[p] = (p == lenquery - 1) ? d : cb[p + 1] + d;
	}


    float *cost;
    float *cost_prev;
    float *cost_tmp;
    int i, j, k;
    float x, z, min_cost;

    /// Instead of using matrix of size O(m^2) or O(mr), we will reuse two array of size O(r).
    cost = (float*)_align_malloc(sizeof(float)*(2*r+1));

    for (k = 0; k < 2*r+1; k++)
		cost[k] = FLT_MAX;

    cost_prev = (float*)_align_malloc(sizeof(float)*(2*r+1));

    for (k = 0; k < 2*r+1; k++)
		cost_prev[k] = FLT_MAX;

    cost_prev[r] = dist(QUERY[0], SUBSMATRIX[t][0]);
    k = r + 1;
    for (j = 1; j <= min(lenquery-1, r); j++, k++)
        cost_prev[k] = cost_prev[k-1] + dist(QUERY[0], SUBSMATRIX[t][j]);

    for (i = 1; i < lenquery; i++)
    {
        k = max(0, r-i);
        min_cost = FLT_MAX;

#pragma ivdep
        for (j = max(0, i-r); j <= min(lenquery-1, i+r); j++, k++)
        {
            if (k+1 > 2*r)
                x = FLT_MAX;
            else
                x = cost_prev[k+1];
            if (j-1 < 0)
                z = FLT_MAX;
            else
                z = cost_prev[k];

            /// Classic DTW calculation
            cost[k] = min(x, z);
        }

        k = max(0, r-i);
        for (j = max(0, i-r); j <= min(lenquery - 1, i + r); j++, k++) {
            if ((j-1 < 0) || ( k-1 < 0))     
				cost[k] = cost[k] + dist(QUERY[i], SUBSMATRIX[t][j]);
            else                      
				cost[k] = min(cost[k], cost[k - 1]) + dist(QUERY[i], SUBSMATRIX[t][j]);

            // Find minimum cost in row for early abandoning (possibly to use column instead of row).
            if (cost[k] < min_cost)
            {   
                min_cost = cost[k];
            }

        }
        
        /// We can abandon early if the current cummulative distance with lower bound together are larger than bsf
        if (i + r < lenquery - 1 && min_cost + cb[i + r + 1] >= bsf)
        {   
            _align_free(cost);
            _align_free(cost_prev);
	    float cb_num = cb[i + r + 1];
	    _align_free(cb);
            return min_cost + cb_num;
        }

        /// Move current array to previous array.
        cost_tmp = cost;
        cost = cost_prev;
        cost_prev = cost_tmp;
    }
    k--;

    /// the DTW distance is in the last cell in the matrix of size O(m^2) or at the middle of our array.
    float final_dtw = cost_prev[k];
    _align_free(cost);
    _align_free(cost_prev);
    _align_free(cb);

    return final_dtw;
}

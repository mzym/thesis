/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Calculation of envelopes for LB_Keogh and LB_Keogh_EC lower bounds.
(c) Mikhail Zymbler 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "system.h"
#include <malloc.h>
#include "envelope.h"

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))


float** UPPER;
float** LOWER;
float* LOWER_q;
float* UPPER_q;


void envelope_query(int lenquery, int r, float matrix[128][205])
{
    for (int i = 0; i < r; i++)
	for (int j = 0; j < (r - i); j++)
	    matrix[i][j] = QUERY[0];
     
	for (int i = lenquery-r; i < lenquery; i++)
	for (int j = r+1; j <= 2*r; j++)
		matrix[i][j] = QUERY[lenquery-1];	    


     int p = 0;
     for (int i = 0; i <= r; i++) {
         p = 0;
         for (int j = r-i; j < lenquery; j++)
            matrix[j][i] = QUERY[p++];
      }

	for (int i = 2*r; i > r; i--) {
		p = lenquery - 1;
		for (int j = lenquery + r - i - 1; j >= 0; j--)
			matrix[j][i] = QUERY[p--];
			
	}	
	   
    for (int i = 0; i < lenquery; i++) {
	float max_value = matrix[i][0];
	float min_value = matrix[i][0];
	for (int j = 0; j <= 2*r; j++) {
	    min_value = min(min_value, matrix[i][j]);
	    max_value = max(max_value, matrix[i][j]);
	}
	UPPER_q[i] = max_value;
	LOWER_q[i] = min_value;
    }
}


void envelope_subsequence(int lenquery, int r, int t)
{
    
    float matrix[128][205] __attribute__((aligned(64)));

    for (int i = 0; i < r; i++)
	for (int j = 0; j < (r - i); j++)
	    matrix[i][j] = SUBSMATRIX[t][0];
	     
   
	for (int i = lenquery-r; i < lenquery; i++)
 	    for (int j = r+1; j <= 2*r; j++)
		matrix[i][j] = SUBSMATRIX[t][lenquery-1];
	   
     int p = 0;
     for (int i = 0; i <= r; i++) {
         p = 0;
         for (int j = r-i; j < lenquery; j++)
            matrix[j][i] = SUBSMATRIX[t][p++];
     }

	for (int i = 2*r; i > r; i--) {
		p = lenquery - 1;
		for (int j = lenquery + r - i - 1; j >= 0; j--)
			matrix[j][i] = SUBSMATRIX[t][p--];
	}

 
    for (int i = 0; i < lenquery; i++) {
	float  max_value = matrix[i][0];
	float  min_value = matrix[i][0];
	for (int j = 0; j <= 2*r; j++) {
	    min_value = min(min_value, matrix[i][j]);
	    max_value = max(max_value, matrix[i][j]);
	}
	UPPER[t][i] = max_value;
	LOWER[t][i] = min_value;
    }
    
}

void init_matrix(int lenquery) {
	UPPER_q = (float*)_align_malloc(lenquery * sizeof(float));
	LOWER_q = (float*)_align_malloc(lenquery * sizeof(float));
	UPPER = (float**)_align_malloc(CountSubsInFragment* sizeof(float*));
	LOWER = (float**)_align_malloc(CountSubsInFragment* sizeof(float*));
	for (int i = 0; i < CountSubsInFragment; i++) {
	    UPPER[i] = (float*)_align_malloc(lenquery * sizeof(float));
	    LOWER[i] = (float*)_align_malloc(lenquery * sizeof(float));
	}
	
}	

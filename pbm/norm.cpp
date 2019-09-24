/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Z-normalization of subsequence.
(c) Mikhail Zymbler 
*/

#include "reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "system.h"

void normalize_query(int lenquery)
{
	float ex = 0, ex2 = 0;
	double  mean = 0, std = 0;

	for (int i = 0; i < lenquery; i++)
	{
		ex += QUERY[i];
		ex2 += QUERY[i] * QUERY[i];
	}

	mean = ex/lenquery;
	std = ex2/lenquery;
	std = sqrt(std - mean * mean);

	for (int i = 0; i < lenquery; i++){
		QUERY[i] = float((QUERY[i] - mean) / std);
	}
}


void normalize(int lenquery, int t)
{
	float ex = 0, ex2 = 0;
	double  mean = 0, std = 0;
	
	for (int i = 0; i < lenquery; i++)
	{
		ex += SUBSMATRIX[t][i];
		ex2 += SUBSMATRIX[t][i] * SUBSMATRIX[t][i];
	}

	mean = ex/lenquery;
	std = ex2/lenquery;
	std = sqrt(std - mean * mean);

	for (int i = 0; i < lenquery; i++) {
		SUBSMATRIX[t][i] = float((SUBSMATRIX[t][i] - mean) / std);
	}

}

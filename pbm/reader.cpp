/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Data reader.
(c) Mikhail Zymbler 
*/

#include "reader.h"
#include "debugger.h"
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "system.h"
#include <cmath>
#include <iostream>

float* QUERY;		// Query
float** SUBSMATRIX; //matrix of subsequences
int CountSubsInFragment;

using namespace std;

void read_timeseries(char* filename, int lenquery, int size, int rank)
{	
	MPI_File TSfile;
	MPI_Offset fragmentStart;
	MPI_Offset fragmentEnd;
	MPI_Offset fileSize;

	int fragmentLength;
	int countSubsInTS;
	int overlap;
	float* fragment;
	
	//open file for all processes in MPI_COMM_WORLD	
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &TSfile);

	//Size of the file in bytes
	MPI_File_get_size(TSfile, &fileSize);
	fileSize = fileSize/sizeof(float); //length of time series
	countSubsInTS = fileSize - lenquery + 1;
	fragmentLength = (int)ceilf((float)countSubsInTS/size);
	overlap = lenquery - 1;
	fragmentStart = fragmentLength * rank;
	fragmentEnd = fragmentStart + fragmentLength + overlap - 1;
	if (rank == size-1) {
		fragmentEnd = fileSize - 1;
	}
	
	fragmentLength = fragmentEnd - fragmentStart + 1;
	
	fragment = (float*)_align_malloc(fragmentLength * sizeof(float));

	MPI_File_read_at_all(TSfile, fragmentStart*sizeof(float), fragment, fragmentLength, MPI_FLOAT, MPI_STATUS_IGNORE);
		
	MPI_File_close(&TSfile);
	
	CountSubsInFragment = fragmentLength - lenquery + 1;

	SUBSMATRIX = (float**)_align_malloc(CountSubsInFragment * sizeof(float*));

	for (int i = 0; i < CountSubsInFragment; i++)
		SUBSMATRIX[i] = (float*)_align_malloc(lenquery * sizeof(float));

	for (int j = 0; j < lenquery; j++)
		SUBSMATRIX[0][j] = fragment[j];

	for (int i = 1; i < CountSubsInFragment; i++) 
	{
		for (int j = 0; j < lenquery-1; j++)
			SUBSMATRIX[i][j] = SUBSMATRIX[i-1][j+1];
		 SUBSMATRIX[i][lenquery-1] = fragment[i+lenquery-1];
	}
	
	_align_free(fragment);
	

	FINISH("Read time series and fill matrixes of subsequences for each node");
}


void read_query(char* filename, int lenquery, int rank)			
{
	MPI_File queryfile;
	MPI_Status status;
	MPI_Offset filesize;
	
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &queryfile);
	MPI_File_get_size(queryfile, &filesize); /* in bytes */
	printf("filesize in bytes = %d\n", filesize);

	QUERY = (float*)_align_malloc(lenquery * sizeof(float));

	MPI_File_read_all(queryfile, QUERY, lenquery, MPI_FLOAT, &status);

	MPI_File_close(&queryfile);

	FINISH("Read query");
}

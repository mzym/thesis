/* (c) Mikhail Zymbler */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include "omp.h"
#include "discords.h"
#include "globals.h"
#include "system.h"

ts_index_t TIME_SERIES_LEN;
ts_index_t __n; // Length of discord
ts_element_t * T; // Time series
ts_index_t discord_loc; // Starting point of real discord in time series
ts_index_t discord_length;
ts_index_t bsf_loc, bruteforce_bsf_loc; // Starting point of discord to be found
ts_distance_t bsf_dist, bruteforce_bsf_dist; // Distance between discord and its nearest neighbor
ts_distance_kind_t D; // Kind of distance measure
double start, finish, start_bruteforce, finish_bruteforce; // Execution time


/*
Generate time series with discord
*/
ts_element_t * generateRandomTimeSeries(void)
{
	T = (ts_element_t *)_align_malloc(TIME_SERIES_LEN * sizeof(ts_element_t));
	assert(T != NULL);

	for (ts_index_t i = 0; i < TIME_SERIES_LEN; i++) {
		T[i] = 2 + cos(i);
	}
	for (ts_index_t i = 0; i < __n; i++) {
		if (i % 2 == 0) {
			T[discord_loc + i] += 719;
		}
		else {
			T[discord_loc + i] -= 313;
		}
	}
	return T;
}

ts_element_t * generateRandomSinTimeSeries(void)
{
	T = (ts_element_t *)_align_malloc(TIME_SERIES_LEN * sizeof(ts_element_t));
	assert(T != NULL);

	for (ts_index_t i = 0; i < TIME_SERIES_LEN; i++) {
		T[i] = 2 + cos(i);
	}
	for (ts_index_t i = 0; i < discord_length; i++) {
		if (i % 2 == 0) {
			T[discord_loc + i] += 719;
		}
		else {
			T[discord_loc + i] -= 313;
		}
	}
	return T;
}

ts_element_t * generateRandomSinTimeSeries(long _discordLength)
{
	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomSinTimeSeries();
}

ts_element_t * generateRandomSinTimeSeries(long m, long _n, long _discordLength)
{
	TIME_SERIES_LEN = m;
	assert(TIME_SERIES_LEN > 0);
	__n = _n;
	assert(__n > 0 && __n < TIME_SERIES_LEN);
	discord_length = _discordLength;
	assert(discord_length > 0 && discord_length < TIME_SERIES_LEN);

	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomSinTimeSeries();
}

ts_element_t * generateRandomExpTimeSeries(void)
{
	T = (ts_element_t *)_align_malloc(TIME_SERIES_LEN * sizeof(ts_element_t));
	assert(T != NULL);

	for (ts_index_t i = 0; i < TIME_SERIES_LEN; i++) {
		T[i] = 2 + exp(i * (__n / TIME_SERIES_LEN));
	}
	for (ts_index_t i = 0; i < discord_length; i++) {
		if (i % 2 == 0) {
			T[discord_loc + i] += 719;
		}
		else {
			T[discord_loc + i] -= 313;
		}
	}
	return T;
}

ts_element_t * generateRandomExpTimeSeries(long _discordLength)
{
	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomExpTimeSeries();
}

ts_element_t * generateRandomExpTimeSeries(long m, long _n, long _discordLength)
{
	TIME_SERIES_LEN = m;
	assert(TIME_SERIES_LEN > 0);
	__n = _n;
	assert(__n > 0 && __n < TIME_SERIES_LEN);
	discord_length = _discordLength;
	assert(discord_length > 0 && discord_length < TIME_SERIES_LEN);

	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomExpTimeSeries();
}

ts_element_t * generateRandomLinearTimeSeries(void)
{
	T = (ts_element_t *)_align_malloc(TIME_SERIES_LEN * sizeof(ts_element_t));
	assert(T != NULL);

	for (ts_index_t i = 0; i < TIME_SERIES_LEN; i++) {
		T[i] = 2 + i;
	}
	for (ts_index_t i = 0; i < discord_length; i++) {
		if (i % 2 == 0) {
			T[discord_loc + i] += 719;
		}
		else {
			T[discord_loc + i] -= 313;
		}
	}
	return T;
}

ts_element_t * generateRandomLinearTimeSeries(long _discordLength)
{
	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomLinearTimeSeries();
}

ts_element_t * generateRandomLinearTimeSeries(long m, long _n, long _discordLength)
{
	TIME_SERIES_LEN = m;
	assert(TIME_SERIES_LEN > 0);
	__n = _n;
	assert(__n > 0 && __n < TIME_SERIES_LEN);
	discord_length = _discordLength;
	assert(discord_length > 0 && discord_length < TIME_SERIES_LEN);

	// Generate time series with random discord's starting point
	srand(time(NULL));
	discord_loc = rand() % (TIME_SERIES_LEN - discord_length);
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
	return generateRandomLinearTimeSeries();
}

ts_element_t * addDiscordsToTimeSeries(ts_element_t * T, long m, long discordLength)
{
	return NULL;
}

/*
Set parameters (manually or get them from command line).
*/
void SetParameters(void)
{
	//TIME_SERIES_LEN = 1 << 10;
	TIME_SERIES_LEN = 1024;
	assert(TIME_SERIES_LEN > 0);

	//n = 1 << 5;
	__n = 64;
	assert(__n > 0 && __n < TIME_SERIES_LEN);

	D = EUQLID2;
	assert(D == EUQLID2);

	discord_length = __n;
	// Generate time series with random discord's starting point
		srand(time(NULL));
		discord_loc = rand() % (TIME_SERIES_LEN - __n);
	// discord_loc = TIME_SERIES_LEN - n;
	assert(discord_loc >= 0 && discord_loc <= TIME_SERIES_LEN - __n);
}

/*
Make and output report on run.
*/
void MakeReport(void)
{
	printf("== Report on run ==\n");
	printf("-- Time series:\n");
	for (ts_index_t i = 0; i < TIME_SERIES_LEN; i++) {
		printf("%lld\t%.16g\n", i, T[i]);
	}
	printf("-- Parameters:\nLength of time series: %lld\nDiscord length: %lld\nDistance measure: %d\nDiscord starts at: %lld\n", TIME_SERIES_LEN, __n, D, discord_loc);
	printf("-- Heuristic search results:\nDiscord starts at: %lld (deviation is %lld)\nDiscord's nearest neighbor distance: %.16g\nExecution time: %.16g\n", bsf_loc, bsf_loc-discord_loc, bsf_dist, finish - start);
	printf("-- Brute force search results:\nDiscord starts at: %lld (deviation is %lld)\nDiscord's nearest neighbor distance: %.16g\nExecution time: %.16g\n", bruteforce_bsf_loc, bsf_loc - discord_loc, bruteforce_bsf_dist, finish_bruteforce - start_bruteforce);
	printf("== End of report ==\n");
}

/*
Heuristic search of discord in a given time series.
Input:
T -- time series
n -- length of discord
Output:
bsf_dist -- distance between discord and its nearest neighbor
Returns:
starting point of the discord in T (numbering starts with 0)
*/
ts_index_t DiscordSearch(const ts_element_t * _T, const ts_index_t n, ts_distance_t * bsf_dist)
{
	ts_index_t p, q;
	ts_distance_t d, nearest_neighbor_dist;

	*bsf_dist = 0;
	bsf_loc = (ts_index_t) NULL;

	for (p = 0; p < TIME_SERIES_LEN - n; p++) {
		nearest_neighbor_dist = INFTY;
		for (q = 0; q < TIME_SERIES_LEN - n; q++) {
			if (abs(p - q) >= n) {
				d = Distance(_T, n, p, q, D);
				assert(d >= 0);
				if (d < *bsf_dist) {
					break;					
				}
				if (d < nearest_neighbor_dist) {
					nearest_neighbor_dist = d;
				}
			}
		}
		if (nearest_neighbor_dist > *bsf_dist) {
			*bsf_dist = nearest_neighbor_dist;
			bsf_loc = p;
		}
	}
	return bsf_loc;
}

/*
Brute force search of discord in a given time series.
Input:
T -- time series
n -- length of discord
Output:
bsf_dist -- distance between discord and its nearest neighbor
Returns:
starting point of the discord in T (numbering starts with 0)
*/
ts_index_t BruteForceDiscordSearch(const ts_element_t * _T, const ts_index_t n, ts_distance_t * bsf_dist)
{
	ts_index_t p, q;
	ts_distance_t d, nearest_neighbor_dist;

	*bsf_dist = 0;
	bsf_loc = (ts_index_t) NULL;

	for (p = 0; p < TIME_SERIES_LEN - n; p++) {
		nearest_neighbor_dist = INFTY;
		for (q = 0; q < TIME_SERIES_LEN - n; q++) {
			if (abs(p - q) >= n) {
				d = Distance(_T, n, p, q, D);
				assert(d >= 0);
				if (d < nearest_neighbor_dist) {
					nearest_neighbor_dist = d;
				}
			}
		}
		if (nearest_neighbor_dist > *bsf_dist) {
			*bsf_dist = nearest_neighbor_dist;
			bsf_loc = p;
		}
	}
	return bsf_loc;
}


/*
Calculate distance between subsequences in a given time series.
Input:
T -- time series
n -- length of subsequences
p -- starting point of the first subsequence
q -- starting point of the second subsequence
D -- kind of the distance (EUQLID2, MANHATTAN, etc.)
Returns:
distance between subsequences
*/
ts_distance_t Distance(const ts_element_t * _T, const ts_index_t n, const ts_index_t p, const ts_index_t q, ts_distance_kind_t D)
{
	ts_distance_t d = 0;

	assert((p>=0) && (p<=TIME_SERIES_LEN-n));
	assert((q>=0) && (q<=TIME_SERIES_LEN-n));
	assert(abs(p-q)>=n);
	switch (D)
	{
	case EUQLID2:
		for (ts_index_t i = 0; i < n; i++) {
			d += ((_T[p+i] - _T[q+i])*(_T[p+i] - _T[q+i]));
		}
		break;
	case MANHATTAN:
		assert(FALSE);
		break;
	default:
		assert(FALSE);
		break;
	}
	return d;
}

/* (c) Mikhail Zymbler */

#ifndef DISCORDS_H
#define DISCORDS_H

typedef float ts_distance_t;
typedef float ts_element_t;
//typedef unsigned long long ts_index_t;
typedef int ts_index_t;
typedef ts_element_t * ts_timeseries_t;

typedef enum distance_kind {
	EUQLID2 = 0, 
	MANHATTAN = 1
} ts_distance_kind_t;

/* Global data. */
/* Parameters */
extern ts_index_t TIME_SERIES_LEN;
extern ts_index_t __n; // Length of discord
extern ts_element_t * T; // Time series
extern ts_index_t discord_loc; // Starting point of real discord in time series
extern ts_index_t bsf_loc, bruteforce_bsf_loc; // Starting point of discord to be found
extern ts_distance_t bsf_dist, bruteforce_bsf_dist; // Distance between discord and its nearest neighbor
extern ts_distance_kind_t D; // Kind of distance measure
extern double start, finish, start_bruteforce, finish_bruteforce; // Execution time

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
ts_index_t DiscordSearch(const ts_element_t * T, const ts_index_t n, ts_distance_t * bsf_dist);

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
ts_index_t BruteForceDiscordSearch(const ts_element_t * T, const ts_index_t n, ts_distance_t * bsf_dist);

/*
Calulate distance between subsequences in a given time series.
Input:
T -- time series
n -- length of subsequences
p -- starting point of the first subsequence
q -- starting point of the second subsequence
D -- kind of the distance (EUQLID2, MANHATTAN, etc.)
Returns:
distance between p and q
*/
ts_distance_t Distance(const ts_element_t * T, const ts_index_t n, const ts_index_t p, const ts_index_t q, ts_distance_kind_t D);

void SetParameters(void);

/*
Generate time series with discord
*/
ts_element_t * generateRandomTimeSeries(void);

ts_element_t * generateRandomSinTimeSeries(void);

ts_element_t * generateRandomSinTimeSeries(long discordLength);

ts_element_t * generateRandomSinTimeSeries(long m, long n,  long discordLength);

ts_element_t * generateRandomExpTimeSeries(void);

ts_element_t * generateRandomExpTimeSeries(long discordLength);

ts_element_t * generateRandomExpTimeSeries(long m, long n, long discordLength);

ts_element_t * generateRandomLinearTimeSeries(void);

ts_element_t * generateRandomLinearTimeSeries(long discordLength);

ts_element_t * generateRandomLinearTimeSeries(long m, long n, long discordLength);

ts_element_t * addDiscordsToTimeSeries(ts_element_t * T, long m, long discordLength);
/*
Make and output report on run.
*/
void MakeReport(void);

#endif

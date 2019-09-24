/* (c) Mikhail Zymbler */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "omp.h"
#include "Globals.h"
#include "DiscordsRun.h"
#include "ArrayUtils.h"
#include "SAX.h"

int bsfPos;
float bsfDist;
series_t timeSeries;
long countOfSubseq;

/**
* Нахождение диссонанса заданной длины в данном временном ряде
* для заданной подпоследовательности
* @input T - временной ряд
* @input m - длина временного ряда
* @input n - длина подпоследовательности
* @output bsf_dist - расстояние до ближайшего соседа
* @input threadNum - кол-во потоков, на которых запускается алгоритм
* @output time - время затраченное на выполнение алгоритма
* @return индекс начала диссонанса
* @return индекс начала диссонанса
*/
int findDiscord(series_t T, const int m, const int n, float* bsf_dist, int threadNum, double* time)
{
	countOfSubseq = m - n + 1;
	_threadNum = threadNum;
	_time = time;
	// normalize
	train(T, m);
	T = normalize(T, m);
	// create matrix of subsequencies
	matrix_t timeSeriesSubsequences = createSubsequencies(T, m, n);
	word* words = (word*)__align_malloc((countOfSubseq) * sizeof(word));
	for (long i = 0; i < countOfSubseq; i++)
	{
		words[i] = (word)__align_malloc((m_string_size) * sizeof(symbol));
	}

	long* minValIndexes = (long*)__align_malloc((countOfSubseq) * sizeof(long));
	long minValIndexesCount = 0;
	// indexes for corresponding to sax words subsequences
	long**wordsTable = generateWordsTable();
	long** chainsOfIndexes = (long**)__align_malloc(powl(m_alphabet_size, m_string_size) * sizeof(long*));
	for (long i = 0; i < powl(m_alphabet_size, m_string_size); i++)
	{
		chainsOfIndexes[i] = (long*)__align_malloc((countOfSubseq) * sizeof(long));
	}

	// prepare
	double start = omp_get_wtime();
	#pragma omp parallel for num_threads(threadNum) shared(timeSeriesSubsequences, words, chainsOfIndexes, wordsTable)
	for (long i = 0; i < countOfSubseq; i++)
	{
		word saxWord = saxify(timeSeriesSubsequences[i], n);
		words[i] = saxWord;
		long index = hashWord(saxWord);
		chainsOfIndexes[index][wordsTable[index][m_string_size]] = i;
		#pragma omp atomic 
		wordsTable[index][m_string_size]++;
	}

	// find min indexes and put it in array
	long minFrequencyValue = MAX_LONG;
	for (long i = 0; i < powl(m_alphabet_size, m_string_size); i++)
	{
		if (wordsTable[i][m_string_size] < minFrequencyValue && wordsTable[i][m_string_size] > 0)
		{
			minFrequencyValue = wordsTable[i][m_string_size];
		}
	}
	for (long i = 0; i < powl(m_alphabet_size, m_string_size); i++)
	{
		if (wordsTable[i][m_string_size] == minFrequencyValue)
		{
			for (long j = 0; j < minFrequencyValue; j++)
			{
				minValIndexes[minValIndexesCount] = chainsOfIndexes[i][j];
				minValIndexesCount++;
			}
		}
	}
	sortIndexes(minValIndexes, minValIndexesCount);

	bsfDist = 0;
	// first outer circle will not be parallelized because minValIndexesCount < threadNum
	for (long i = 0; i < minValIndexesCount; i++)
	{
		double min_val = POS_INF;
		bool earlyExit = false;
		long currentMinValIndex = minValIndexes[i];
		word currentMinValSaxWord = words[currentMinValIndex];
		long chainIndex = hashWord(currentMinValSaxWord);
		long innerIterationsCount = wordsTable[chainIndex][m_string_size];
		#pragma omp parallel num_threads(threadNum) shared(minValIndexes, chainsOfIndexes, timeSeriesSubsequences)
		#pragma omp for schedule(dynamic, 1) reduction(min:min_val)
		for (long j = 0; j < innerIterationsCount; j++)
		{
			bool selfMatch = isSelfMatch(minValIndexes[i], chainsOfIndexes[chainIndex][j], n);
			if (!selfMatch)
			{
				double dist = distance2(timeSeriesSubsequences[minValIndexes[i]], timeSeriesSubsequences[chainsOfIndexes[chainIndex][j]], n);
				if (dist < bsfDist) {
					#pragma omp atomic write
					earlyExit = true;
					#pragma omp cancel for 
					//break;
				}
				if (dist < min_val) {
						min_val = dist;
				}
			}
		}
		#pragma omp parallel num_threads(threadNum) shared(minValIndexes, chainsOfIndexes, timeSeriesSubsequences)
		#pragma omp for schedule(dynamic, 1) reduction(min:min_val)
		for (long j = 0; j < countOfSubseq; j++)
		{
			bool selfMatch = isSelfMatch(minValIndexes[i], j, n);
			if (!selfMatch)
			{
				long binSearchResult = binSearch(chainsOfIndexes[chainIndex], innerIterationsCount, j);
				if (binSearchResult == -1)
				{
					double dist = distance2(timeSeriesSubsequences[minValIndexes[i]], timeSeriesSubsequences[j], n);
					if (dist < bsfDist) {
						#pragma omp atomic write
						earlyExit = true;
						#pragma omp cancel for
					}
					if (dist < min_val) {
						min_val = dist;
					}
				}
			}
		}
		if (!earlyExit && min_val > bsfDist) {
			bsfDist = min_val;
			bsfPos = minValIndexes[i];
		}
	}

	#pragma omp parallel for num_threads(threadNum) shared(minValIndexes, chainsOfIndexes, timeSeriesSubsequences)
	for (long i = 0; i < countOfSubseq; i++)
	{
		long binSearchResult = binSearch(minValIndexes, minValIndexesCount, i);
		if (binSearchResult == -1)
		{
			double min = POS_INF;
			bool earlyExit = false;
			long chainIndex = hashWord(words[i]);
			long innerIterationsCount = wordsTable[chainIndex][m_string_size];

			for (long j = 0; j < innerIterationsCount; j++)
			{
				bool selfMatch = isSelfMatch(i, chainsOfIndexes[chainIndex][j], n);
				if (!selfMatch)
				{
					double dist = distance2(timeSeriesSubsequences[i], timeSeriesSubsequences[chainsOfIndexes[chainIndex][j]], n);
					if (dist < bsfDist) {
						earlyExit = true;
						break;
					}
					if (dist < min) {
						min = dist;
					}
				}
			}
			for (long j = 0; j < countOfSubseq; j++)
			{	
				bool selfMatch = isSelfMatch(i, j, n);
				if (!selfMatch)
				{
					binSearchResult = binSearch(chainsOfIndexes[chainIndex], innerIterationsCount, j);
					if (binSearchResult == -1)
					{
						double dist = distance2(timeSeriesSubsequences[i], timeSeriesSubsequences[j], n);
						if (dist < bsfDist) {
							earlyExit = true;
							break;
						}
						if (dist < min) {
							min = dist;
						}
					}
				}
			}
			if (!earlyExit && min > bsfDist) {
				#pragma omp critical
				{
					bsfDist = min;
					bsfPos = i;
				}
			}
		}
	}
	double end = omp_get_wtime();
	*_time += (end - start);
	*bsf_dist = bsfDist;
	return bsfPos;
}

/**
* Создание матрицы подпоследовательностей
* для заданной подпоследовательности
* @param T - временной ряд
* @param m - длина временного ряда
* @param n - длина подпоследовательности
* @return матрица подпоследовательностей
*/
matrix_t createSubsequencies(const series_t T, const int m, const int n)
{
	matrix_t result = (matrix_t)__align_malloc((m - n + 1) * sizeof(series_t));
	assert(result != NULL);
	for (int i = 0; i < m - n + 1; i++)
	{
		result[i] = (series_t)__align_malloc(n * sizeof(item_t));
		assert(result[i] != NULL);
	}
	for (int i = 0; i < m - n + 1; i++)
	{
		memcpy(result[i], &T[i], n * sizeof(item_t));
	}
	return result;
}

/*
* Are sequences with given start indexes self match?
* @param i - start index of first sequence
* @param j - start index of second sequence
* @param n - length of sequences
* @return are self match or not
*/
bool isSelfMatch(long i, long j, long n)
{
	return !(j <= i - n || j >= i + n);
}

/**
* Calculates the square of the Euclidean distance between two single-dimensional timeseries represented
* by the rational vectors.
* @param point1 The first timeseries.
* @param point2 The second timeseries.
* @param length The length of series.
* @return The Euclidean distance.
*/
item_t distance2(const series_t series1, const series_t series2, const int length)
{
	assert(length > 0);
	assert(series1 != NULL);
	assert(series2 != NULL);
	float sum = 0;
	MY_ASSUME_ALIGNED(series1);
	MY_ASSUME_ALIGNED(series2);
	for (int i = 0; i < length; i++)
	{
		sum += (series1[i] - series2[i]) * (series1[i] - series2[i]);
	}
	return sum;
}


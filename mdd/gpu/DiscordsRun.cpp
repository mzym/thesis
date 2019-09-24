/* (c) Mikhail Zymbler */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
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
    double start;
    #pragma acc data copyin(chainsOfIndexes[0:powl(m_alphabet_size, m_string_size)][0:powl(m_alphabet_size, m_string_size)]), \
            minValIndexes[0:countOfSubseq], \
            wordsTable[0:powl(m_alphabet_size, m_string_size)][0:powl(m_alphabet_size, m_string_size+ 1)]], \
            words[0:countOfSubseq], \
            timeSeriesSubsequences[0:countOfSubseq][0:n], \
            T[0:m], \
            bsfPos, bsfDist, threadNum, n, m) \
        copyout(bsfPos, bsfDist)
    {

        // prepare
        start = omp_get_wtime();
        #pragma acc parallel loop device_type(nvidia)
        for (long i = 0; i < countOfSubseq; i++) {
            word saxWord = saxify(timeSeriesSubsequences[i], n);
            words[i] = saxWord;
            long index = hashWord(saxWord);
            chainsOfIndexes[index][wordsTable[index][m_string_size]] = i;
            #pragma acc atomic
            wordsTable[index][m_string_size]++;
        }

        // find min indexes and put it in array
        long minFrequencyValue = MAX_LONG;
        #pragma acc parallel loop device_type(nvidia)
        for (long i = 0; i < powl(m_alphabet_size, m_string_size); i++) {
            if (wordsTable[i][m_string_size] < minFrequencyValue && wordsTable[i][m_string_size] > 0) {
                #pragma acc atomic
                minFrequencyValue = wordsTable[i][m_string_size];
            }
        }
        #pragma acc parallel loop gang worker device_type(nvidia)
        for (long i = 0; i < powl(m_alphabet_size, m_string_size); i++) {
            if (wordsTable[i][m_string_size] == minFrequencyValue) {
                #pragma acc loop vector(32)
                for (long j = 0; j < minFrequencyValue; j++) {
                    minValIndexes[minValIndexesCount] = chainsOfIndexes[i][j];
                    #pragma acc atomic
                    minValIndexesCount++;
                }
            }
        }
        sortIndexes(minValIndexes, minValIndexesCount);

        bsfDist = 0;
        // first outer circle will not be parallelized because minValIndexesCount < threadNum
        for (long i = 0; i < minValIndexesCount; i++) {
            double min_val = POS_INF;
            volatile bool earlyExit = false;
            long currentMinValIndex = minValIndexes[i];
            word currentMinValSaxWord = words[currentMinValIndex];
            long chainIndex = hashWord(currentMinValSaxWord);
            long innerIterationsCount = wordsTable[chainIndex][m_string_size];

            #pragma acc parallel device_type(nvidia)
            #pragma acc loop gang worker reduction(min:min_val)
            for (long j = 0; j < innerIterationsCount; j++) {
                if(earlyExit) continue;
                bool selfMatch = isSelfMatch(minValIndexes[i], chainsOfIndexes[chainIndex][j], n);
                if (!selfMatch) {
                    double dist = distance2(timeSeriesSubsequences[minValIndexes[i]],
                                            timeSeriesSubsequences[chainsOfIndexes[chainIndex][j]], n);
                    if (dist < bsfDist) {
                        #pragma acc atomic
                        earlyExit = true;
                        continue;
                    }
                    if (dist < min_val) {
                        min_val = dist;
                    }
                }
            }

            #pragma acc parallel if(earlyExit == false) device_type(nvidia)
            #pragma acc loop gang worker reduction(min:min_val)
            for (long j = 0; j < countOfSubseq; j++) {
                bool selfMatch = isSelfMatch(minValIndexes[i], j, n);
                if (!selfMatch) {
                    long binSearchResult = binSearch(chainsOfIndexes[chainIndex], innerIterationsCount, j);
                    if (binSearchResult == -1) {
                        double dist = distance2(timeSeriesSubsequences[minValIndexes[i]], timeSeriesSubsequences[j], n);
                        if (dist < bsfDist) {
                            #pragma acc atomic
                            earlyExit = true;
                            continue;
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

        #pragma acc parallel loop device_type(nvidia)
        for (long i = 0; i < countOfSubseq; i++) {
            long binSearchResult = binSearch(minValIndexes, minValIndexesCount, i);
            if (binSearchResult == -1) {
                double min = POS_INF;
                bool earlyExit = false;
                long chainIndex = hashWord(words[i]);
                long innerIterationsCount = wordsTable[chainIndex][m_string_size];
                for (long j = 0; j < innerIterationsCount; j++) {
                    bool selfMatch = isSelfMatch(i, chainsOfIndexes[chainIndex][j], n);
                    if (!selfMatch) {
                        double dist = distance2(timeSeriesSubsequences[i],
                                                timeSeriesSubsequences[chainsOfIndexes[chainIndex][j]], n);
                        if (dist < bsfDist) {
                            earlyExit = true;
                            break;
                        }
                        if (dist < min) {
                            min = dist;
                        }
                    }
                }
                for (long j = 0; j < countOfSubseq; j++) {
                    bool selfMatch = isSelfMatch(i, j, n);
                    if (!selfMatch) {
                        binSearchResult = binSearch(chainsOfIndexes[chainIndex], innerIterationsCount, j);
                        if (binSearchResult == -1) {
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
                    #pragma acc critical
                    {
                        bsfDist = min;
                        bsfPos = i;
                    }
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
    #pragma acc parallel loop vector_length(128) reduction(+,sum) device_type(nvidia)
	for (int i = 0; i < length; i++)
	{
		sum += (series1[i] - series2[i]) * (series1[i] - series2[i]);
	}
	return sum;
}


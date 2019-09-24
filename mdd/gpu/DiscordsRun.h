/* (c) Mikhail Zymbler */

#ifndef DISCORDSRUN_H
#define DISCORDSRUN_H

#include "Config.h"
#include <iostream>

/*
* Are sequences with given start indexes not self match?
* @param i - start index of first sequence
* @param j - start index of second sequence
* @param n - length of sequences
* @return are self match or not
*/
#define IS_NOT_SELF_MATCH(i, j, n) ( !(j <= i - n || j >= i + n) )
#define IS_SELF_MATCH(i, j, n) ( !(IS_NOT_SELF_MATCH(i, j, n)) )

extern int bsfPos;
extern float bsfDist;
extern series_t timeSeries;

using namespace std;

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
*/
int findDiscord(const series_t T, const int m, const int n, float* bsf_dist, int threadNum, double* time);

/**
* Создание матрицы подпоследовательностей
* для заданной подпоследовательности
* @param T - временной ряд
* @param m - длина временного ряда
* @param n - длина подпоследовательности
* @return матрица подпоследовательностей
*/
matrix_t createSubsequencies(const series_t T, const int m, const int n);


/**
* Calculates the square of the Euclidean distance between two single-dimensional timeseries represented
* by the rational vectors.
* @param point1 The first timeseries.
* @param point2 The second timeseries.
* @param length The length of series.
* @return The Euclidean distance.
*/
item_t distance2(const series_t series1, const series_t series2, const int length);

/*
* @deprecated: use macro instead
* Are sequences with given start indexes self match?
* @param i - start index of first sequence
* @param j - start index of second sequence
* @param n - length of sequences
* @return are self match or not
*/
bool isSelfMatch(long i, long j, long n);

#endif

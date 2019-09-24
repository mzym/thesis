/* (c) Mikhail Zymbler */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <fstream> //Для работы с файлами
#include "Config.h"

using namespace std;

extern char* BASE_DIR;
extern char* TIME_SERIES_FILE_NAME;
extern char* RESULT_FILE_NAME;
extern char* DISTANCE_MATRIX_FILE_NAME;

/**
 * Data structure:
 * 1. TIME_SERIES_FILE
 * first string: length of series - long
 * second string: time series items:
 * 2. DISTANCE_MATRIX_FILE
 * first string: m-n+1 - long
 * strings 2 - m-n+2: distance matrix rows:
 */

series_t readTimeSeries(ifstream reader);

matrix_t readDistanceMatrix(ifstream reader);

bool writeResult(ofstream writer, long bsfPos, float bsfDist);

bool writeResult(long bsfPos, float bsfDist, float time, int threadsNum);

bool writeDistanceMatrix(ofstream writer, matrix_t distMatrix);

/**
 * Read time series items from text file
 * File contains m items (one per line)
 * @param m - length of time series
 * @param path - path to file with time series data
 * @return time series
 */
series_t readTimeSeries(char* path, int m);

#endif

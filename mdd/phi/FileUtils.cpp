/* (c) Mikhail Zymbler */

#pragma warning(disable:4996)
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "FileUtils.h"
#include "Config.h"

#pragma warning(disable:4996)

using namespace std;

char* BASE_DIR = "/data/";
char* TIME_SERIES_FILE_NAME = "timeSeries.bin";
char* RESULT_FILE_NAME = "result.bin";
char* DISTANCE_MATRIX_FILE_NAME = "matrix.bin";

series_t readTimeSeries(ifstream reader) {
	int length;
	series_t memblock = NULL;
	char * fullPath = "";
	strcpy(fullPath, BASE_DIR);
	strcat(fullPath, TIME_SERIES_FILE_NAME);
	reader.open(fullPath, ios::binary);
	if (reader.is_open())
	{
		streampos size;
		size = reader.tellg();
		reader.seekg(0, ios::beg);
		reader.read(reinterpret_cast<char*>(&length), sizeof(int));
		memblock = new float[length];
		item_t item;
		int i = 0;
		while (reader.read(reinterpret_cast<char*>(&item), sizeof(item_t)))
		{
			memblock[i] = item;
			i++;
		}
		reader.close();
	}
	assert(memblock != NULL);
	return memblock;
}

series_t readTimeSeries(char* path, int m)
{
	ifstream fin(path);
	series_t series = (item_t*)__align_malloc((m) * sizeof(item_t));
	string str;
	int i = 0;
	while (getline(fin, str) && i < m)
	{
		series[i] = stof(str);
		i++;
	}
	return series;
}

matrix_t readDistanceMatrix(ifstream reader) {
	// todo
	return 0;
}

bool writeResult(ofstream writer, long bsfPos, float bsfDist) {
	// todo
	return true;
}

bool writeResult(long bsfPos, float bsfDist, float time, int threadsNum) {
	ofstream fout("result.txt", ios_base::out | ios_base::app);
	fout << bsfPos << " " << bsfDist << " " << time << " " << threadsNum << "\n";
	fout.close(); // закрываем файл
	return true;
}

bool writeDistanceMatrix(ofstream writer, matrix_t distMatrix) {
	// todo
	return true;
}

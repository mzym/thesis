/* (c) Mikhail Zymbler */

#ifndef SAX_H
#define SAX_H

#include "Config.h"

extern size_t m_string_size;
extern size_t m_alphabet_size;
extern float m_baseline_mean;
extern float m_baseline_stdev;

/*
* Create SAX representation of given subsequnce of time series
* Input: sequence and it's size
*/
word saxify(series_t timeSeries, const int n);

/*
* Finds the baseline mean and stdevs, which are used in
* normalizing the input time series.
* Input: timeSeries and it's size
*/
void train(series_t timeSeries, const long size);

/*
* Normalize time series to zero mean and stdev
* Input: timeSeries and it's size
*/
series_t normalize(series_t timeSeries, const long size);

/*
* Create PAA approximation of given time series subsequence
* Input: timeSeries and it's size
*/
series_t PAA(series_t sequence, const int n);

/*
 * Map SAX word to it's index in array of words
 * Input: saxWord - word
 * Return: hash
 */
long hashWord(word saxWord);

/*
* generate words table (such as trie but array)
*/
long** generateWordsTable();

#endif

/* (c) Mikhail Zymbler */

#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

/*
* Binary search in given array
* Return: index of element or -1
*/
long binSearch(long* array, long size, long value);

/*
* Sort array with indexes in asc order
* Input: array and it's length
* Output: sorted array
*/
void sortIndexes(long* array, long n);

#endif

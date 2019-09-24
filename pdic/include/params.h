/*
Parallel Dynamic Itemset Counting.
Parameters of the algorithm.

(c) 2016 Mikhail Zymbler
*/

#ifndef PARAMS_H
#define PARAMS_H

// Parameters of datasets
#define n_RECORDLINK	((unsigned long long) 574912)
#define m_RECORDLINK	((unsigned long long) 29)

#define n_SKIN	((unsigned long long) 240000)
#define m_SKIN	((unsigned long long) 11)

#define n_RETAIL	((unsigned long long) 88163)
#define m_RETAIL	((unsigned long long) 16500)

#define n_ACCIDENTS	((unsigned long long) 340184)
#define m_ACCIDENTS	((unsigned long long) 572)

#define n_SUSY	((unsigned long long) 5000000)
#define m_SUSY	((unsigned long long) 190)

#define n_WEBDOCS	((unsigned long long) 1692082)
#define m_WEBDOCS	((unsigned long long) 5267656)

// Parameters of serial algorithm
#define n_MAX	((unsigned long long) 10000000) // Number of transactions
#define m_MAX	m_SUSY // Number of items

#endif
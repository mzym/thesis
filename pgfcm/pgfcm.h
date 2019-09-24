/* pgfcm.h
 * Fuzzy c-Means clustering in PargreSQL.
 * Interface.
 * (c) 2019 Mikhail Zymbler
 */ 
 
#ifndef _PGFCM_H_
#define _PGFCM_H_

#include <par_libpq-fe.h>
#include <time.h>

/* 
Input:
	conn	- connection to PargreSQL server
	d		- dimension
	k		- number of clusters
	m		- fuzzyness (typically m=2)
	eps		- stopping criterion (typically eps=0.001)
	itermax	- maximal number of iterations to perform clustering
	SH		- name of a table in database with k+1 numeric columns where columns have titles "i" and "x1", "x2", ..., "xk"
	SV		- name of a table in database with k numeric columns where columns have titles "x1", "x2", ..., "xk"
Output:
	None
Side effect(s):	
	Creates in database table U with memberships of each data point
Returns:
	Run time of the algorithm
 */
clock_t pgfcm(PGconn *conn, int d, int k, float m, float eps, int itermax, char *SH, char *SV); 

#endif


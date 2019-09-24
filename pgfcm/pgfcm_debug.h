/* pgfcm.h
 * Debugger for Fuzzy c-Means clustering in PargreSQL.
 * Interface.
 * (c) 2019 Mikhail Zymbler
 */ 
 
#ifndef _PGFCM_DEBUG_H_
#define _PGFCM_DEBUG_H_

#include <stdio.h>
#include <string.h>
#include <par_libpq-fe.h>

/* This macro is to turn debug mode on (DEBUG) or off (NDEBUG)*/
#define NDEBUG	

#ifdef NDEBUG	
/* Output a message */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DBG(msg, ...) do { fprintf(stderr, "DEBUG %s:%d " msg "\n", __FILENAME__, __LINE__, ##__VA_ARGS__); fflush(stdout); } while (0); 
#define PRINT(msg, ...) do { fprintf(stderr, msg, ##__VA_ARGS__); fflush(stdout); } while (0);

/* Output START and FINISH messages */
#define START(name, ...) PRINT("\nSTART:\t" name "\n", ##__VA_ARGS__)
#define FINISH(name, ...) PRINT("FINISH:\t" name "\n", ##__VA_ARGS__)

/* Nice assertion of statements for debugging */
#define PG_ASSERT(conn, res, cond, msg)	assert_nicely(conn, res, cond, msg)

/* Print contents of the SH table (data vectors in horizontal form) */
#define PGFCM_SH(conn)	pgfcm_SH(conn)

/* Print contents of the SV table (data vectors in vertical form) */
#define PGFCM_SV(conn)	pgfcm_SV(conn)

/* Print contents of the С table (coordinates of centroids) */
#define PGFCM_C(conn)	pgfcm_C(conn)

/* Print contents of the SD table (distances between x_i and c_j) */
#define PGFCM_SD(conn)	pgfcm_SD(conn)

/* Print contents of the U table (memberships of x_i in c_j cluster at s-th step) */
#define PGFCM_U(conn)	pgfcm_U(conn)

/* Print contents of the UT table (memberships of x_i in c_j cluster at (s+1)-th step) */
#define PGFCM_UT(conn)	pgfcm_UT(conn)

/* Print contents of the P table (result of computations delta at s-th step) */
#define PGFCM_P(conn)	pgfcm_P(conn)

/* Print contents of the R table (temp table, similar to U) */
#define PGFCM_R(conn)	pgfcm_R(conn)

/* Print contents of TMP table for debug reasons */
#define PGFCM_TMP(conn)	pgfcm_tmp(conn)

#else 
#define DBG(msg, ...)	
#define PRINT(msg, ...)	
#define START(name, ...)	
#define FINISH(name, ...)	
#define PG_ASSERT(conn, res, cond, msg)		
#define PGFCM_SH(conn)	
#define PGFCM_SV(conn)	
#define PGFCM_C(conn)	
#define PGFCM_SD(conn)	
#define PGFCM_U(conn)	
#define PGFCM_UT(conn)	
#define PGFCM_P(conn)	
#define PGFCM_R(conn)	
#define PGFCM_TMP(conn)	
#endif

/*---------------------------------------------------------------------------------*/
/* Nice assertion of statements for debugging */
void assert_nicely(PGconn *conn, PGresult *res, int cond, char * msg);

/* Print contents of the SH table (data vectors in horizontal form) */
void pgfcm_SH(PGconn *conn);

/* Print contents of the SV table (data vectors in vertical form) */
void pgfcm_SV(PGconn *conn);

/* Print contents of the С table (coordinates of centroids) */
void pgfcm_C(PGconn *conn);

/* Print contents of the SD table (distances between x_i and c_j) */
void pgfcm_SD(PGconn *conn);

/* Print contents of the U table (memberships of x_i in c_j cluster at s-th step) */
void pgfcm_U(PGconn *conn);

/* Print contents of the UT table (memberships of x_i in c_j cluster at (s+1)-th step) */
void pgfcm_UT(PGconn *conn);

/* Print contents of the P table (result of computations delta at s-th step) */
void pgfcm_P(PGconn *conn);

/* Print contents of the R table (temp table, similar to U) */
void pgfcm_R(PGconn *conn);

/* Print contents of TMP table for debug reasons */
void pgfcm_tmp(PGconn *conn);
/*---------------------------------------------------------------------------------*/

#endif


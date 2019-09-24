/* (c) Mikhail Zymbler */
 
#ifndef _PGFCM_DEBUG_H_
#define _PGFCM_DEBUG_H_

#include <libpq-fe.h>

void assert_nicely(PGconn *conn, PGresult *res, int cond, char * msg);

void lp_GRAPH(PGconn *conn);

void lp_VERTEX(PGconn *conn);

void lp_AFFINITY(PGconn *conn);

void lp_COMMUNITY(PGconn *conn);

void lp_AFF_TMP_SUBG(PGconn *conn);

void lp_AFF_TMP_WNBR(PGconn *conn);

void lp_COMM_TMP_AFNBRALL(PGconn *conn);

void lp_COMM_TMP_AFNBRCOM(PGconn *conn);

void lp_COMM_TMP_DMAX(PGconn *conn);

void lp_VER_TMP_COMM(PGconn *conn);

void lp_tmp(PGconn *conn);

#endif


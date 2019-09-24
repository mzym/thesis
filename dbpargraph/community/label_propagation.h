/* 
Detection graph communities with Parallel DBMS.
(c) Mikhail Zymbler 
*/
 
#ifndef _PGFCM_H_
#define _PGFCM_H_

#include <par_libpq-fe.h>


void label_propagation(PGconn *conn, int F, char *GRAPH, char *VERTEX);

void assert_nicely(PGconn *conn, PGresult *res, int cond, char *msg);
#endif

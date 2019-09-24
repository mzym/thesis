/*-----------------------------------------------------------------------------
 *
 * par_libpq-fe.h
 * 	This file contains implementation for functions used by frontend
 * 	pargres applications.
 *
 * 2010, Constantin S. Pan
 *
 *-----------------------------------------------------------------------------
 */

#define PAR_NO_COMPAT
#define PAR_CONFIG_FILENAME "par_libpq.conf"

#include "par_libpq-fe.h"
#include "libpq-int.h"
#include "par_config.h"
#include <stdlib.h>

par_PGconn *par_PQconnectdb(void)
{
	int i;
	par_config *conf = par_config_load(PAR_CONFIG_FILENAME);

	par_PGconn *conn = malloc(sizeof(par_PGconn));
	conn->len = conf->nodes_count;
	conn->conns = malloc(conn->len * sizeof(PGconn*));
	for (i = 0; i < conn->len; i++)
	{
		conn->conns[i] = PQconnectdb(conf->conninfo[i]);
	}

	par_config_unload(conf);

	return conn;
}

void par_PQfinish(par_PGconn *conn)
{
	int i;
	for (i = 0; i < conn->len; i++) {
		PQfinish(conn->conns[i]);
	}
}

ConnStatusType par_PQstatus(const par_PGconn *conn)
{
	int i;
	for (i = 0; i < conn->len; i++) {
		if (PQstatus(conn->conns[i]) != CONNECTION_OK)
		{
			printf("connection %d (%s) is not OK, returned %d (%s)\n",
					i,
					conn->conns[i]->pghost,
					PQstatus(conn->conns[i]),
					PQerrorMessage(conn->conns[i])
			);
			return CONNECTION_BAD;
		}
	}
	return CONNECTION_OK;
}

PGresult *par_PQexec(par_PGconn *conn, const char *query)
{
	int i;
	PGresult *r;
	for (i = 1; i < conn->len; i++) {
		//PGresult *ignore = PQexec(conn->conns[i], query);
		//PQclear(ignore);
		PQsendQuery(conn->conns[i], query); // asynchronous command
	}
	r = PQexec(conn->conns[0], query); // synchronous command
	for (i = 1; i < conn->len; i++) {
		PGresult *ignore;
		while ((ignore = PQgetResult(conn->conns[i])) != NULL) {
			PQclear(ignore);
		}
	}
	return r;
}

#undef PAR_NO_COMPAT

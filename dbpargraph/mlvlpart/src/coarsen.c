/* 
Graph partitioning with Parallel DBMS: coarsening phase.
(c) Mikhail Zymbler 
*/

#include <stdio.h>
#include <stdlib.h>
#include <par_libpq-fe.h> // For plain PostgreSQL try 'libpq-fe.h'
//#include <libpq-fe.h>

static void exit_nicely(PGconn *conn) {
	PQfinish(conn);
	exit(1);
}

int main(int argc, char **argv) {
	const char *conninfo;
	char query[1024];
	char *tablename;
	char *matchingname;
	char *coarsename;
	PGconn *conn;
	PGresult *res;
	int i, j;
	float w;

	/*
	 * This config-string only matters in plain PostgreSQL.
	 * PargreSQL uses a separate file 'par_libpq.conf' instead,
	 * where strings like this should be listed.
	 */
	if (argc != 5) {
		fprintf(stderr, "Need three arguments: conninfo, fine tablename, matching tablename, coarse tablename\n");
		exit(1);
	}
	//conninfo = "dbname=postgres hostaddr=10.1.11.2 port=5432 user=mzym password=pass";
	conninfo = argv[1];
	tablename = argv[2];
	matchingname = argv[3];
	coarsename = argv[4];

	/* Make a connection to the database */
	fprintf(stderr, "Making a connection to the database\n");
	conn = PQconnectdb(conninfo);
	fprintf(stderr, "Checking the connection status\n");

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed\n");
		exit_nicely(conn);
	}

	res = PQexec(conn, "set enable_pargresql=0;");
	/*
	 * Check the result status.
	 * For SELECT queries this should normally be PGRES_TUPLES_OK.
	 * For UPDATE, DELETE and DDL queries - PGRES_COMMAND_OK.
	 */
	if (PQresultStatus(res) != PGRES_COMMAND_OK) { // Something's wrong...
		fprintf(stderr, "Disabling PargreSQL failed\n");
		PQclear(res);
		exit_nicely(conn);
	} else {
		fprintf(stderr, "PargreSQL disabled!\n");
	}

	/* Send the queries! */
	sprintf(query, "select coarsen('%s', '%s', '%s');", tablename, matchingname, coarsename);
	fprintf(stderr, "Sending query: %s\n", query);
	res = PQexec(conn, query);
	/*
	 * Check the result status.
	 * For SELECT queries this should normally be PGRES_TUPLES_OK.
	 * For UPDATE, DELETE and DDL queries - PGRES_COMMAND_OK.
	 */
	if (PQresultStatus(res) != PGRES_TUPLES_OK) { // Something's wrong...
		fprintf(stderr, "Query execution failed\n");
		PQclear(res);
		exit_nicely(conn);
	} else {
		fprintf(stderr, "Queries executed!\n");
	}

	///* First, print out the attribute names */
	int nFields = PQnfields(res);
	for (i = 0; i < nFields; i++) {
		printf("%-15s", PQfname(res, i));
	}
	printf("\n\n");

	/* Next, print out the tuples */
	for (i = 0; i < PQntuples(res); i++) {
		for (j = 0; j < nFields; j++) {
			printf("%-15s,", PQgetvalue(res, i, j));
		}
		printf("\n");
	}

	PQclear(res);

	/* Close the connection to the database and cleanup */
	PQfinish(conn);

        return 0;
}

/* 
Export graph partitions to CSV.
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
	PGconn *conn;
	PGresult *res;
	int nFields;
	int i, j;
	int edgenum = 0, vertnum = 0;

	/*
	 * This config-string only matters in plain PostgreSQL.
	 * PargreSQL uses a separate file 'par_libpq.conf' instead,
	 * where strings like this should be listed.
	 */
	if (argc != 3) {
		fprintf(stderr, "Need two arguments: conninfo and tablename\n");
		exit(1);
	}
	//conninfo = "dbname=postgres hostaddr=10.1.11.2 port=5432 user=mzym password=pass";
	conninfo = argv[1];
	tablename = argv[2];

	/* Make a connection to the database */
	conn = PQconnectdb(conninfo);

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed\n");
		exit_nicely(conn);
	}

	/* Send the queries! */
	sprintf(query, "select a, b, w from %s;", tablename);
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
	nFields = PQnfields(res);
	//for (i = 0; i < nFields; i++) {
	//	printf("%-15s", PQfname(res, i));
	//}
	//printf("\n\n");

	/* Next, print out the tuples */
	for (i = 0; i < PQntuples(res); i++) {
		for (j = 0; j < nFields - 1; j++) {
			printf("%s,", PQgetvalue(res, i, j));
		}
		printf("%s", PQgetvalue(res, i, j));
		printf("\n");
	}

	PQclear(res);

        /* Close the connection to the database and cleanup */
        PQfinish(conn);

        return 0;
}

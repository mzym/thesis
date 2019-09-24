/* pgfcm_debug.c
 * Debugger for Fuzzy c-Means clustering in PargreSQL.
 * Implementation.
 * (c) 2019 Mikhail Zymbler
 */ 

#include "pgfcm_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <par_libpq-fe.h>

/* Nice assertion of statemants for debugging */
void assert_nicely(PGconn *conn, PGresult *res, int cond, char * msg)
{
	if (!cond) {
		fprintf(stderr, "%s\n", msg);
		if (res != NULL) {
		    fprintf(stderr, "Server error message is as follows:\n%s\n", PQresultErrorMessage(res));
		    PQclear(res);
		}
		PQfinish(conn);
		exit(1);
	}
}

/* Print contents of the SH table (data vectors in horizontal form) */
void pgfcm_SH(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from SH order by i;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the SH table failed!");	
	fprintf(stderr, "SH:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the SV table (data vectors in vertical form) */
void pgfcm_SV(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from SV order by i,l;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the SV table failed!");	
	fprintf(stderr, "SV:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the С table (coordinates of centroids) */
void pgfcm_C(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select j, l, round(val::numeric,2) from C order by j,l;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the C table failed!");	
	fprintf(stderr, "C:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the SD table (distances between x_i and c_j) */
void pgfcm_SD(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select i, j, round(dist::numeric,10) as dist from SD order by i,j;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the C table failed!");	
	fprintf(stderr, "SD:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the U table (memberships of x_i in c_j cluster at s-th step) */
void pgfcm_U(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select i, j, round(val::numeric,2) as val from U order by i,j;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the U table failed!");	
	fprintf(stderr, "U:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the UT table (memberships of x_i in c_j cluster at (s+1)-th step) */
void pgfcm_UT(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select i, j, round(val::numeric,2) as val from UT order by i,j;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the UT table failed!");	
	fprintf(stderr, "UT:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of the P table (result of computations delta at s-th step) */
void pgfcm_P(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select d, k, n, s, round(delta::numeric,10) as delta, runtime from P order by s;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the P table failed!");	
	fprintf(stderr, "P:\n");
	PQprint(stderr, res, &opt);			
}

void IMPORT_SH_TABLE(PGconn *conn)
{
        PQexec(conn, "COPY SH FROM '/home/pangres/pargresql-tests/test-pgfcm/SUSY_SH_TABLE.csv' DELIMITER ',' CSV HEADER;");
}

void IMPORT_SV_TABLE(PGconn *conn)
{
        PQexec(conn, "COPY SH FROM '/home/pangres/pargresql-tests/test-pgfcm/SUSY_SV_TABLE.csv' DELIMITER ',' CSV HEADER;");
}

void SHOW_SH_SV_TABLES_ROWS(PGconn *conn)
{
        PGresult *res;
        PQprintOpt opt={0};
        opt.header = 1;
        opt.align = 1;
        opt.fieldSep = "|";

        res = PQexec(conn, "select count(*) from SH;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the SH table failed!");
        fprintf(stderr, "SH:\n");
        PQprint(stderr, res, &opt);

        res = PQexec(conn, "select count(*) from SV;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the SV table failed!");
        fprintf(stderr, "SV:\n");
        PQprint(stderr, res, &opt);

}

/* Print contents of the R table (temp table, similar to U) */
void pgfcm_R(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from R order by i,j;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the R table failed!");	
	fprintf(stderr, "R:\n");
	PQprint(stderr, res, &opt);			
}

/* Print contents of TMP table for debug reasons */
void pgfcm_tmp(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from tmp;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the TMP table failed!");	
	fprintf(stderr, "TMP:\n");
	PQprint(stderr, res, &opt);			
}



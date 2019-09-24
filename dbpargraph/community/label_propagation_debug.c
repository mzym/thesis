/* (c) Mikhail Zymbler */

#include "label_propagation_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

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

void lp_GRAPH(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from GRAPH order by a, b;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the GRAPH table failed!");
	fprintf(stderr, "GRAPH:\n");
	PQprint(stderr, res, &opt);			
}

void lp_VERTEX(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

    res = PQexec(conn, "select * from VERTEX order by a;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the VERTEX table failed!");
	fprintf(stderr, "VERTEX:\n");
	PQprint(stderr, res, &opt);			
}

void lp_AFFINITY(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from AFFINITY order by a,b;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the AFFINITY table failed!");	
	fprintf(stderr, "AFFINITY:\n");
	PQprint(stderr, res, &opt);			
}

void lp_COMMUNITY(PGconn *conn)
{
    PGresult *res;
    PQprintOpt opt={0};
    opt.header = 1;
    opt.align = 1;
    opt.fieldSep = "|";
    
    res = PQexec(conn, "select * from COMMUNITY order by a;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the COMMUNITY table failed!");
    fprintf(stderr, "COMMUNITY:\n");
    PQprint(stderr, res, &opt);
}

void lp_AFF_TMP_SUBG(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from AFF_TMP_SUBG order by a,b;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the AFF_TMP_SUBG table failed!");
	fprintf(stderr, "AFF_TMP_SUBG:\n");
	PQprint(stderr, res, &opt);			
}

void lp_AFF_TMP_WNBR(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from AFF_TMP_WNBR order by a;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the AFF_TMP_WNBR table failed!");
	fprintf(stderr, "AFF_TMP_WNBR:\n");
	PQprint(stderr, res, &opt);			
}

void lp_COMM_TMP_AFNBRALL(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from COMM_TMP_AFNBRALL order by a;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the COMM_TMP_AFNBRALL table failed!");
	fprintf(stderr, "COMM_TMP_AFNBRALL:\n");
	PQprint(stderr, res, &opt);			
}

void lp_COMM_TMP_AFNBRCOM(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from COMM_TMP_AFNBRCOM order by a;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the COMM_TMP_AFNBRCOM table failed!");
	fprintf(stderr, "COMM_TMP_AFNBRCOM:\n");
	PQprint(stderr, res, &opt);			
}

void lp_COMM_TMP_DMAX(PGconn *conn)
{
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";

	res = PQexec(conn, "select * from COMM_TMP_DMAX order by a;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the COMM_TMP_DMAX table failed!");
	fprintf(stderr, "COMM_TMP_DMAX:\n");
	PQprint(stderr, res, &opt);			
}

void lp_VER_TMP_COMM(PGconn *conn)
{
    PGresult *res;
    PQprintOpt opt={0};
    opt.header = 1;
    opt.align = 1;
    opt.fieldSep = "|";
    
    res = PQexec(conn, "select * from VER_TMP_COMM order by a;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples from the VER_TMP_COMM table failed!");
    fprintf(stderr, "VER_TMP_COMM:\n");
    PQprint(stderr, res, &opt);
}

void lp_tmp(PGconn *conn)
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

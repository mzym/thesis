/* 
Detection graph communities with Parallel DBMS.
(c) Mikhail Zymbler 
*/

#include "label_propagation.h"
#include "label_propagation_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <par_libpq-fe.h>
#include <time.h>

void label_propagation(PGconn *conn, int F, char *GRAPH, char *VERTEX)
{
    int stop = 0, c = 0;
	PGresult *res;
    int v = 0;
    double time_start, time_stop;
    
    res = PQexec(conn, "SELECT count(*)\
               FROM VERTEX;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the vertex");
    v = atoi(PQgetvalue(res, 0, 0));
    printf("%d\n", v);
    
    fprintf(stderr, "Create AFFINITY table.\n");
    res = PQexec(conn, "drop table if exists affinity;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table AFFINITY if exists failed!");
    res = PQexec(conn, "create table affinity (a int, b int, afty numeric, primary key (a,b));");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create AFFINITY table failed!");

    fprintf(stderr, "Create COMMUNITY table.\n");
    res = PQexec(conn, "drop table if exists community;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table COMMUNITY if exists failed!");
    res = PQexec(conn, "create table community (a int, c int, d numeric, primary key (a,c));");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMMUNITY table failed!");
    
	fprintf(stderr, "Initialization phase.\n");
    fprintf(stderr, "Create AFF_TMP_SUBG temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE AFF_TMP_SUBG (a int, b int, w numeric, PRIMARY KEY (a,b));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create AFF_TMP_SUBG table failed!");
    
    fprintf(stderr, "Create AFF_TMP_WNBR temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE AFF_TMP_WNBR (a int, wnbr numeric, PRIMARY KEY (a));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create AFF_TMP_WNBR table failed!");
    
    fprintf(stderr, "Create COMM_TMP_AFNBRALL temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE COMM_TMP_AFNBRALL (a int, afnbrall numeric, PRIMARY KEY (a));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMM_TMP_AFNBRALL table failed!");
    
    fprintf(stderr, "Create COMM_TMP_AFNBRCOM temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE COMM_TMP_AFNBRCOM (a int, c int, afnbrcom numeric, PRIMARY KEY (a,c));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMM_TMP_AFNBRCOM table failed!");
    
    fprintf(stderr, "Create COMM_TMP_DMAX temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE COMM_TMP_DMAX (a int, dmax numeric, PRIMARY KEY (a));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMM_TMP_DMAX table failed!");
    
    fprintf(stderr, "Create VER_TMP_COMM temp table.\n");
    res = PQexec(conn, "CREATE TEMP TABLE VER_TMP_COMM (a int, c int, PRIMARY KEY (a));");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create VER_TMP_COMM table failed!");
		
    fprintf(stderr, "Insert into VER_TMP_COMM temp table.\n");
	res = PQexec(conn, "INSERT INTO AFF_TMP_SUBG\
                 SELECT a, b, w FROM GRAPH\
                 UNION\
                 SELECT b, a, w FROM GRAPH;")
    
    fprintf(stderr, "Insert into AFF_TMP_WNBR temp table.\n");
    res = PQexec(conn, "INSERT INTO AFF_TMP_WNBR\
                 SELECT a, sum(w) FROM AFF_TMP_SUBG\
                 GROUP BY a;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into AFF_TMP_WNBR table failed!");
    
    fprintf(stderr, "Insert into AFFINITY temp table.\n");
    res = PQexec(conn, "INSERT INTO AFFINITY\
                 SELECT X.a, b, w/wnbr\
                 FROM AFF_TMP_SUBG AS X, AFF_TMP_WNBR AS Y\
                 WHERE X.a = Y.a;");
    lp_AFFINITY(conn);
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into AFFINITY table failed!");
    
    fprintf(stderr, "Insert into COMM_TMP_AFNBRALL temp table.\n");
    res = PQexec(conn, "INSERT INTO COMM_TMP_AFNBRALL\
                 SELECT a, sum(afty)\
                 FROM AFFINITY\
                 GROUP BY a;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMM_TMP_AFNBRALL table failed!");
    
    int i = 0;
    
    while (!stop) {
        time_start = clock();
        
        fprintf(stderr, "Insert into COMM_TMP_AFNBRCOM temp table.\n");
        res = PQexec(conn, "INSERT INTO COMM_TMP_AFNBRCOM\
                     SELECT AFFINITY.a, VERTEX.c, sum(afty)\
                     FROM AFFINITY, VERTEX\
                     WHERE AFFINITY.b = VERTEX.a\
                    GROUP BY AFFINITY.a, VERTEX.c;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMM_TMP_AFNBRCOM table failed!");
        
        fprintf(stderr, "Insert into COMMUNITY temp table.\n");
        res = PQexec(conn, "INSERT INTO COMMUNITY\
                     SELECT X.a, c, afnbrcom/afnbrall AS d\
                     FROM COMM_TMP_AFNBRALL AS X, COMM_TMP_AFNBRCOM AS Y\
                     WHERE X.a = Y.a;");
        lp_COMMUNITY(conn);
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMMUNITY table failed!");
        
        fprintf(stderr, "Insert into COMM_TMP_DMAX temp table.\n");
        res = PQexec(conn, "INSERT INTO COMM_TMP_DMAX\
                     SELECT a, max(d) AS d\
                     FROM COMMUNITY\
                     GROUP BY a;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COM_TMP_DMAX table failed!");
    
  
        fprintf(stderr, "Insert into VER_TMP_COMM temp table.\n");
        res = PQexec(conn, "INSERT INTO VER_TMP_COMM\
                     SELECT a, min(c)\
                     FROM (SELECT X.a, X.c\
                            FROM COMMUNITY AS X, COMM_TMP_DMAX AS Y\
                            WHERE X.a = Y.a AND X.d = Y.dmax) as P\
                     group by a;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COM_TMP_COM table failed!");
        
        res = PQexec(conn, "SELECT count(*)\
                     FROM VER_TMP_COMM;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the coincidence");
        printf("size of VER_TMP_COMM = %d\n", atoi(PQgetvalue(res, 0, 0)));
        
        res = PQexec(conn, "SELECT count(*)*2\
                     FROM VERTEX, VER_TMP_COMM\
                     WHERE VERTEX.a = VER_TMP_COMM.a AND VERTEX.c = VER_TMP_COMM.c;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the coincidence");
        int s = atoi(PQgetvalue(res, 0, 0));
        printf("stability = %.2f %%\n", (float)50*s/v);
        printf("s = %d\n", s/2);
        
        res = PQexec(conn, "UPDATE VERTEX\
                     SET c = (SELECT c\
                     FROM VER_TMP_COMM\
                     WHERE VER_TMP_COMM.a = VERTEX.a);");
        lp_VERTEX(conn);
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update VERTEX table failed!");
        
        res = PQexec(conn, "SELECT count(distinct c)\
                     FROM VERTEX;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the coincidence");
        c = atoi(PQgetvalue(res, 0, 0));
        printf("c = %d\n", c);
        
        res = PQexec(conn, "TRUNCATE COMM_TMP_AFNBRCOM;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate COMM_TMP_AFNBRCOM table failed!");
        res = PQexec(conn, "TRUNCATE COMMUNITY;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate COMMUNITY table failed!");
        res = PQexec(conn, "TRUNCATE COMM_TMP_DMAX;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate COMM_TMP_DMAX table failed!");
        res = PQexec(conn, "TRUNCATE VER_TMP_COMM;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate VER_TMP_COMM table failed!");
        if (s > v)
            stop = 1;
        else
            if (c <= F)
                stop = 1;
        
        time_stop = clock();
        fprintf(stderr, "i = %d, Loop Runtime is %.2f.\n", i, time_stop - time_start);
        i++;
    }
}

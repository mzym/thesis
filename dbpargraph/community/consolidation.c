/* 
Consolidation of graph communities with Parallel DBMS.
(c) Mikhail Zymbler 
*/

#include "consolidation.h"
#include "label_propagation_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <par_libpq-fe.h>
#include <time.h>

void consolidation(PGconn *conn, int F, char *GRAPH, char *VERTEX)
{
    int c = 0, current_comm_count = 0, c1 = 0, c2 = 0, e = 0;
	PGresult *res;
    double time_start, time_stop;
    char *query;
    
	fprintf(stderr, "Initialization phase.\n");
    fprintf(stderr, "Create NEW_COMMUNITY temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE NEW_COMMUNITY (c_new int, c_old int, PRIMARY KEY (c_new));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create NEW_COMMUNITY table failed!");
    
    fprintf(stderr, "Create COMMUNITY_EDGES temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE COMMUNITY_EDGES (c int, edges int, PRIMARY KEY (c));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMMUNITY_EDGES table failed!");
    
    fprintf(stderr, "Create COMMUNITY_VERTEX temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE COMMUNITY_VERTEX (comm_a int, comm_b int);");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create COMMUNITY_VERTEX table failed!");
    
    fprintf(stderr, "Create PAIRS_COMMUNITY temp table.\n");
	res = PQexec(conn, "CREATE TEMP TABLE PAIRS_COMMUNITY (c1 int, c2 int, cut_edges int, PRIMARY KEY (c1,c2));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create PAIRS_COMMUNITY table failed!");
    
    res = PQexec(conn, "SELECT count(distinct c)\
                 FROM VERTEX;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the community");
    comm_count = atoi(PQgetvalue(res, 0, 0));
    printf("count of communities after the label propagation = %d\n", comm_count);
    current_comm_count = comm_count;
    
    fprintf(stderr, "Insert into NEW_COMMUNITY temp table.\n");
    res = PQexec(conn, "INSERT INTO NEW_COMMUNITY\
                 SELECT row_number() over(), c\
                 FROM (SELECT c\
                    FROM VERTEX\
                    GROUP BY c\
                    ORDER BY c) AS comm;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into NEW_COMMUNITY table failed!");
    
    res = PQexec(conn, "UPDATE VERTEX\
                 SET c = (SELECT c_new\
                 FROM NEW_COMMUNITY\
                 WHERE NEW_COMMUNITY.c_old = VERTEX.c);");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update VERTEX table failed!");
    
    fprintf(stderr, "Insert into COMMUNITY_EDGES temp table.\n");
    res = PQexec(conn, "INSERT INTO COMMUNITY_EDGES\
                 SELECT c_new, 0\
                 FROM NEW_COMMUNITY;");
    lp_AFFINITY(conn);
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMMUNITY_EDGES table failed!");
    
    fprintf(stderr, "Insert into COMMUNITY_VERTEX temp table.\n");
    res = PQexec(conn, "INSERT INTO COMMUNITY_VERTEX (comm_a, comm_b)\
                 SELECT v1.c, v2.c\
                 FROM  (SELECT VERTEX.c, row_number() over() AS rn\
                            from VERTEX, GRAPH\
                            WHERE graph.a=VERTEX.a) AS v1\
                        FULL OUTER JOIN\
                            (SELECT VERTEX.c, row_number() over() AS rn\
                            FROM VERTEX, GRAPH\
                            WHERE GRAPH.b=VERTEX.a) AS v2\
                        ON v1.rn=v2.rn;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMMUNITY_VERTEX table failed!");
    
    res = PQexec(conn, "UPDATE COMMUNITY_EDGES\
                 SET edges = (SELECT count\
                 FROM (SELECT comm_a, count(*) as count\
                        FROM COMMUNITY_VERTEX\
                        WHERE comm_a=comm_b\
                        GROUP BY comm_a) as COMM\
                 WHERE COMM.comm_a=COMMUNITY_EDGES.c);");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update COMMUNITY_EDGES table failed!");
    
    fprintf(stderr, "Insert into PAIRS_COMMUNITY temp table.\n");
    res = PQexec(conn, "INSERT INTO PAIRS_COMMUNITY\
                 SELECT *, count(*) as count\
                 FROM COMMUNITY_VERTEX\
                 WHERE comm_a!=comm_b\
                 GROUP BY comm_a, comm_b\
                 ORDER BY count;");
    lp_AFFINITY(conn);
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into PAIRS_COMMUNITY table failed!");
    
    while (current_comm_count > F) {
        
        res = PQexec(conn, "SELECT c1, c2, (e1+cut_edges+COMMUNITY_EDGES.edges) AS total_edges\
                     FROM (SELECT c1, c2, edges AS e1, cut_edges\
                        FROM COMMUNITY_EDGES, PAIRS_COMMUNITY\
                        WHERE c1=c) AS comm, COMMUNITY_EDGES\
                        WHERE comm.c2=COMMUNITY_EDGES.c\
                        ORDER BY total_edges, c1, c2\
                        LIMIT 1;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "The pair of communities which are necessary to be united");
        c1 = atoi(PQgetvalue(res, 0, 0));
        c2 = atoi(PQgetvalue(res, 0, 1));
        e = atoi(PQgetvalue(res, 0, 2));
        printf("The pair of communities = %d %d, edges = %d\n", c1, c2, e);
        
        comm_count++;
        
        asprintf(&query, "UPDATE VERTEX SET c = %d where c=%d or c=%d;", comm_count, c1, c2);
        
        res = PQexec(conn, query);
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update VERTEX table failed!");
        
        res = PQexec(conn, "TRUNCATE COMMUNITY_VERTEX;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate COMMUNITY_VERTEX table failed!");
        res = PQexec(conn, "TRUNCATE COMMUNITY_EDGES;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate COMMUNITY_EDGES table failed!");
        res = PQexec(conn, "TRUNCATE PAIRS_COMMUNITY;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate PAIRS_COMMUNITY table failed!");
        
        fprintf(stderr, "Insert into COMMUNITY_VERTEX temp table.\n");
        res = PQexec(conn, "INSERT INTO COMMUNITY_VERTEX (comm_a, comm_b)\
                     SELECT v1.c, v2.c\
                     FROM  (SELECT VERTEX.c, row_number() over() AS rn\
                     from VERTEX, GRAPH\
                     WHERE graph.a=VERTEX.a) AS v1\
                     FULL OUTER JOIN\
                     (SELECT VERTEX.c, row_number() over() AS rn\
                     FROM VERTEX, GRAPH\
                     WHERE GRAPH.b=VERTEX.a) AS v2\
                     ON v1.rn=v2.rn;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMMUNITY_VERTEX table failed!");
        
        res = PQexec(conn, "UPDATE COMMUNITY_VERTEX\
                     SET comm_a = comm_b, comm_b = comm_a\
                     WHERE comm_a>comm_b;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update COMMUNITY_VERTEX table failed!");
        
        res = PQexec(conn, "INSERT INTO COMMUNITY_EDGES\
                     SELECT distinct c, 0\
                     FROM VERTEX;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COMMUNITY_EDGES table failed!");
        
        res = PQexec(conn, "UPDATE COMMUNITY_EDGES\
                     SET edges = (SELECT count\
                     FROM (SELECT comm_a, count(*) as count\
                     FROM COMMUNITY_VERTEX\
                     WHERE comm_a=comm_b\
                     GROUP BY comm_a) as COMM\
                     WHERE COMM.comm_a=COMMUNITY_EDGES.c);");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update COMMUNITY_EDGES table failed!");
        
        fprintf(stderr, "Insert into PAIRS_COMMUNITY temp table.\n");
        res = PQexec(conn, "INSERT INTO PAIRS_COMMUNITY\
                     SELECT *, count(*) as count\
                     FROM COMMUNITY_VERTEX\
                     WHERE comm_a!=comm_b\
                     GROUP BY comm_a, comm_b\
                     ORDER BY count;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into PAIRS_COMMUNITY table failed!");
        
        current_comm_count--;
    }
    
    res = PQexec(conn, "TRUNCATE NEW_COMMUNITY;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate NEW_COMMUNITY table failed!");
    
    fprintf(stderr, "Insert into NEW_COMMUNITY temp table.\n");
    res = PQexec(conn, "INSERT INTO NEW_COMMUNITY\
                 SELECT row_number() over(), c\
                 FROM (SELECT c\
                 FROM VERTEX\
                 GROUP BY c\
                 ORDER BY c) AS comm;");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into NEW_COMMUNITY table failed!");
    
    res = PQexec(conn, "UPDATE VERTEX\
                 SET c = (SELECT c_new\
                 FROM NEW_COMMUNITY\
                 WHERE NEW_COMMUNITY.c_old = VERTEX.c);");
    assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update VERTEX table failed!");
    
    res = PQexec(conn, "ALTER TABLE GRAPH ADD COLUMN f int;");
    
    time_start = clock();
    
    while (!stop) {
        
        fprintf(stderr, "Insert into COMM_TMP_AFNBRCOM temp table.\n");
        res = PQexec(conn, "INSERT INTO COMM_TMP_AFNBRCOM\
                     SELECT AFFINITY.a, VERTEX.c, sum(afty)\
                     FROM AFFINITY, VERTEX\
                     WHERE AFFINITY.b = VERTEX.a\
                    GROUP BY AFFINITY.a, VERTEX.c;");
        lp_COMM_TMP_AFNBRCOM(conn);
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
        lp_COMM_TMP_DMAX(conn);
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COM_TMP_DMAX table failed!");

        fprintf(stderr, "Insert into VER_TMP_COMM temp table.\n");
        res = PQexec(conn, "INSERT INTO VER_TMP_COMM\
                     SELECT a, min(c)\
                     FROM (SELECT X.a, X.c\
                            FROM COMMUNITY AS X, COMM_TMP_DMAX AS Y\
                            WHERE X.a = Y.a AND X.d = Y.dmax) as P\
                     group by a;");
        lp_VER_TMP_COMM(conn);
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into COM_TMP_COM table failed!");
        
        res = PQexec(conn, "SELECT count(*)*2\
                     FROM VERTEX, VER_TMP_COMM\
                     WHERE VERTEX.a = VER_TMP_COMM.a AND VERTEX.c = VER_TMP_COMM.c;");
        assert_nicely(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Calculate count of the coincidence");
        int s = atoi(PQgetvalue(res, 0, 0));
        printf("s = %d\n", s);
        
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
        printf("c = %d\n", c)
        
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
            if (c <= c_max)
                stop = 1;
    }
    
    time_stop = clock();
    fprintf(stderr, "Loop Runtime is %.2f.\n", time_stop - time_start);
}

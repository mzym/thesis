/* 
Graph communities detectio with Parallel DBMS.
(c) Mikhail Zymbler 
*/

#include "label_propagation.h"
#include "consolidation.h"
#include "label_propagation_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <par_libpq-fe.h>
#include <time.h>

int main(int argc, char **argv) {
    
    char* fname = argv[1]; // count of communities per node, "graph.csv"
    int F = atoi(argv[2]); // count of nodes
    double runtime, start, end;
    
	const char *conninfo;
	PGconn *conn;
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";
	conninfo = "host=localhost dbname=graph port=5433 user=mzym password=qwerty";

	fprintf(stderr, "Make a connection to the database.\n");
	conn = PQconnectdb(conninfo);
	assert_nicely(conn, NULL, PQstatus(conn)==CONNECTION_OK, "Connection to database failed");
    
	fprintf(stderr, "Create GRAPH table.\n");
	res = PQexec(conn, "drop table if exists graph;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table GRAPH if exists failed!");
	res = PQexec(conn, "create table graph (a int, b int, w numeric, primary key (a,b));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create GRAPH table failed!");
	
	printf("Fill in GRAPH table.\n");
    
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error reading file\n");
        return 1;
    }
    
    int vertex1 = 0, vertex2 = 0;
    float weight = 0.0f;
    char *query;
    
    while (fscanf(fp, "%d,%d,%f", &vertex1, &vertex2, &weight) == 3) {
        asprintf(&query, "insert into graph values (%d,%d,%f);", vertex1, vertex2, weight);
        
        res = PQexec(conn, query); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in GRAPH table failed!");
    }

	fprintf(stderr, "Create VERTEX table.\n");
	res = PQexec(conn, "drop table if exists vertex;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table VERTEX if exists failed!");
	res = PQexec(conn, "create table vertex (a int, c int, primary key (a));");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create VERTEX table failed!");

	fprintf(stderr, "Fill in VERTEX table.\n");
	res = PQexec(conn, "insert into vertex select a,a from (select a from graph union select b from graph) as vertices;"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in VERTEX table failed!");
    

	fprintf(stderr, "Run butterfly test.\n");
    
    start = clock();
    
	label_propagation(conn, F, "GRAPH", "VERTEX");
	
    end = clock();
    runtime = end - start;
    
    fprintf(stderr, "Full Runtime is %.2f.\n", runtime);
    
	fprintf(stderr, "Results of the butterfly test.\n");
    
    res = PQexec(conn, "SELECT row_number() OVER(), c, count(*)\
                 FROM VERTEX\
                 GROUP BY c;");

    int nrows = PQntuples(res);
    for (int i = 0; i < nrows; i++)
    {
        char* group_number = PQgetvalue(res, i, 0);
        char* label = PQgetvalue(res, i, 1);
        char* count_vertex_in_group = PQgetvalue(res, i, 2);
        printf("group number: %s, label: %s, count vertex in group: %s\n", group_number, label, count_vertex_in_group);
    }
    
    FILE *fvertex=fopen("vertex.csv","w");
    if (fvertex == NULL) {
        fprintf(stderr, "Error writing vertex file\n");
        return 1;
    }
    
    res = PQexec(conn, "SELECT * FROM VERTEX\
                 ORDER BY a;");
    
    int count_vertex = PQntuples(res);
    
    for (int i = 0; i < count_vertex; i++)
    {
        char* vertex = PQgetvalue(res, i, 0);
        char* c = PQgetvalue(res, i, 1);
        fprintf(fvertex, "%s,%s\n", vertex, c);
    }
    
    fclose(fvertex);
    
    consolidation(conn, F, "GRAPH", "VERTEX");
    
	fprintf(stderr, "Close the connection to the database and cleanup.\n");
	PQfinish(conn);

    return 0;
}

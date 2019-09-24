/*
 * This is a simple test for pgFCM algorithm under PargreSQL DBMS. 
 * (c) 2019 Mikhail Zymbler
 */
#include "pgfcm.h" 
#include "pgfcm_debug.h" 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <par_libpq-fe.h> // For plain PostgreSQL try 'libpq-fe.h'

int main(int argc, char **argv) {
	const char *conninfo;
	PGconn *conn;
	PGresult *res;
	PQprintOpt opt={0};
	opt.header = 1;
	opt.align = 1;
	opt.fieldSep = "|";
	conninfo = "";//dbname=postgres hostaddr=10.1.11.2 port=5432 user=pan password=pass";
	double runtime;

	fprintf(stderr, "Make a connection to the database.\n");
	conn = PQconnectdb(conninfo);
	assert_nicely(conn, NULL, PQstatus(conn)==CONNECTION_OK, "Connection to database failed");

	fprintf(stderr, "Create SH table.\n");
	res = PQexec(conn, "drop table if exists sh;"); 
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table SH if exists failed!");
	res = PQexec(conn, "create table sh (i int, x1 numeric,	x2 numeric, primary key (i)) with (fragattr = i);");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SH table failed!");
	
	printf("Fill in SH table.\n");
	res = PQexec(conn, "insert into sh values (1,0,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (2,0,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (3,0,4);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (4,1,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (5,1,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (6,1,3);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (7,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (8,3,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (9,4,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (10,5,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (11,5,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (12,5,3);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (13,6,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (14,6,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	res = PQexec(conn, "insert into sh values (15,6,4);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");		

	pgfcm_SH(conn);

	fprintf(stderr, "Create SV table.\n");
	res = PQexec(conn, "drop table if exists sv;");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table SV if exists failed!");
	res = PQexec(conn, "create table sv (i int, l int, val numeric, primary key (i,l)) with (fragattr = i);");
	assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SV table failed!");		

	fprintf(stderr, "Fill in SV table.\n");
	res = PQexec(conn, "insert into sv values (1,1,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (1,2,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (2,1,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (2,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (3,1,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (3,2,4);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (4,1,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (4,2,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (5,1,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (5,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (6,1,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (6,2,3);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (7,1,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (7,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (8,1,3);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (8,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (9,1,4);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (9,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (10,1,5);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (10,2,1);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (11,1,5);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (11,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (12,1,5);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (12,2,3);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (13,1,6);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (13,2,0);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (14,1,6);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (14,2,2);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (15,1,6);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	res = PQexec(conn, "insert into sv values (15,2,4);"); assert_nicely(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");

	pgfcm_SV(conn);

	fprintf(stderr, "Run butterfly test.\n");
	runtime = pgfcm(conn, 2, 2, 2.0, 0.01, 1000, "SH", "SV");
	fprintf(stderr, "Runtime is %.2f.\n", runtime);
	
	fprintf(stderr, "Results of the butterfly test.\n");
	pgfcm_U(conn);
	
	fprintf(stderr, "Protocol of the butterfly test.\n");
	pgfcm_P(conn);

	fprintf(stderr, "Close the connection to the database and cleanup.\n");
	PQfinish(conn);

    return 0;
}

/*
 * This is a test program for pgFCM algorithm under PargreSQL DBMS. 
 * Command line parameters:
 * 1. number of clusters (positive integer)
 * 2. fuzzyness (positive integer; typically 2)
 * 3. stopping criterion (positive real; typically 0.001)
 * 4. maximal number of iterations (positive integer; typically 1000)
 * 5. CSV file name with ``horizontal'' data
 * 6. CSV file name with ``vertical'' data
 * (c) 2019 Mikhail Zymbler
 */
#undef DEBUG
#define NDEBUG

#include "pgfcm.h" 
#include "pgfcm_debug.h"
#include "csvparser.h"
#include <assert.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <par_libpq-fe.h> // For plain PostgreSQL try 'libpq-fe.h'
#include "omp.h"

int main(int argc, char **argv) 
{
	const char *conninfo;
	PGconn *conn;
	PGresult *res;
	conninfo = "";//dbname=postgres hostaddr=10.1.11.2 port=5432 user=pan password=pass";
	clock_t runtime;
	char qry[2048];
	char tmp[1024];
	char SHname[64];
	char SVname[64];
	char *dataline; 
	int i, d, k, m, itermax;
	float eps;
	CsvParser *csv;
	FILE *f;

	if (argc < 6) {
		printf("Fuzzy clustering inside PargreSQL DBMS:\n");
		printf("No.\tSemantic\n");
		printf("1 k\tnumber of clusters (positive integer)\n");
		printf("2 m\tfuzzyness (positive integer; typically 2)\n");
		printf("3 eps\tstopping criterion (positive real; typically 0.001)\n");
		printf("4 itermax\tmaximal number of iterations (positive integer; typically 1000)\n");
		printf("5 SHname\tCSV file name with ``horizontal'' data\n");
		printf("6 SVname\tCSV file name with ``vertical'' data\n");
		return 0;
	}

	// Get command line parameters
	k = atoi(argv[1]);
	assert(k>0);
	m = atoi(argv[2]);
	assert(m >= 2);
	eps = atof(argv[3]);
	assert(eps > 0 && eps < 1);
	itermax = atoi(argv[4]);
	assert(itermax>0);
	sprintf(SHname, argv[5]);
	sprintf(SVname, argv[6]);

	csv = CsvParser_new(SHname, ",", 0);
	assert(csv!=NULL);
	d = CsvParser_getNumFields(CsvParser_getRow(csv))-1;
	assert(d>1);
	CsvParser_destroy(csv);

	//double start_mp = omp_get_wtime();
	PRINT("Make a connection to the database.\n");
	conn = PQconnectdb(conninfo);
	PG_ASSERT(conn, NULL, PQstatus(conn)==CONNECTION_OK, "Connection to database failed");

	fprintf(stderr, "Create SH table.\n");
	res = PQexec(conn, "DROP TABLE IF EXISTS SH;"); 
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table SH if exists failed!");
	sprintf(qry, "CREATE TABLE SH (i int");
	for (i=1; i<=d; i++) {
		sprintf(tmp, ", x%d real", i);
		strcat(qry, tmp);
	}
	strcat(qry, ", PRIMARY KEY (i)) with (fragattr = i);");
	PRINT("Run `%s'.\n", qry);
	res = PQexec(conn, qry);
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SH table failed!");

	//double end_mp = omp_get_wtime();
	//fprintf(stderr, "Run time is %.10f\n", end_mp - start_mp);	
	fprintf(stderr, "Fill in SH table.\n");
	f = fopen(SHname, "rt");
	assert(f != NULL);
	while (!feof(f)) {
		fscanf(f, "%s\n", dataline);
		sprintf(qry, "insert into SH values (");
		strcat(qry, dataline);
		strcat(qry, ");");
		PRINT("Run `%s'.\n", qry);
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SH table failed!");
	}
	fclose(f);
	PGFCM_SH(conn);

	fprintf(stderr, "Create SV table.\n");
	res = PQexec(conn, "drop table if exists SV;");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Dropping table SV if exists failed!");
	res = PQexec(conn, "create table SV (i int, l int, val real, primary key (i,l)) with (fragattr = i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SV table failed!");		

	fprintf(stderr, "Fill in SV table.\n");
	f = fopen(SVname, "rt");
	assert(f != NULL);
	while (!feof(f)) {
		fscanf(f, "%s\n", dataline);
		sprintf(qry, "insert into SV values (");
		strcat(qry, dataline);
		strcat(qry, ");");
		PRINT("Run `%s'.\n", qry);	
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in SV table failed!");
	}
	fclose(f);
	PGFCM_SV(conn);

	fprintf(stderr, "Run test.\n");
	runtime = pgfcm(conn, d, k, m, eps, itermax, "SH", "SV");
	fprintf(stderr, "Runtime is %.12f.\n", runtime);
	
	//fprintf(stderr, "Results of the test.\n");
	//pgfcm_U(conn);
	
	fprintf(stderr, "Protocol of the test\n");
	pgfcm_P(conn);

	fprintf(stderr, "Close the connection to the database and cleanup.\n");
	PQfinish(conn);

    return 0;
}

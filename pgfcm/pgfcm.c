/* pgfcm.c
 * Fuzzy c-Means clustering in PargreSQL.
 * Implementation.
 * (c) 2019 Mikhail Zymbler
 */ 

#include "pgfcm.h"
#include "pgfcm_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <par_libpq-fe.h>

#define	TRUE (1)

/* 
Input:
	conn	- connection to PargreSQL server
	d		- dimension
	k		- number of clusters
	m		- fuzzyness (typically m=2)
	eps		- stopping criterion (typically eps=0.001)
	itermax	- maximal number of iterations to perform clustering
	SH		- name of a table in database with k+1 real columns where columns have titles "i" and "x1", "x2", ..., "xk"
	SV		- name of a table in database with k real columns where columns have titles "x1", "x2", ..., "xk"
Output:
	None
Side effect(s):	
	Creates in database table U with memberships of each data point
Returns:
	Run time of the algorithm
 */
clock_t pgfcm(PGconn *conn, int d, int k, float m, float eps, int itermax, char *SH, char *SV)
{
	clock_t start, end, start_iter, end_iter;
	char qry[2048];
	float tmp;
	unsigned int steps, icnt, jcnt, n;
	PGresult *res;
	

	struct timespec timer_start(){
    		struct timespec start_time;
    		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    		return start_time;
	}	

	long timer_end(struct timespec start_time){
    		struct timespec end_time;
    		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    		long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
    		return diffInNanos;
	}

	PRINT("Initialization phase.\n"); 
	res = PQexec(conn, "CREATE TEMP TABLE C (j int, l int, val real, PRIMARY KEY (j,l)) with (fragattr=j);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create C table failed!");
	res = PQexec(conn, "CREATE TEMP TABLE SD (i int, j int, dist real, PRIMARY KEY (i,j)) with (fragattr=i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SD table failed!");
	res = PQexec(conn, "CREATE TEMP TABLE U (i int, j int, val real, PRIMARY KEY (i,j)) with (fragattr=i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create U table failed!");
	res = PQexec(conn, "CREATE TEMP TABLE UT (i int, j int, val real, PRIMARY KEY (i,j)) with (fragattr=i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create UT table failed!");
	res = PQexec(conn, "CREATE TEMP TABLE P (d int, k int, n int, s int, delta real, runtime timestamp, PRIMARY KEY (d,k,n,s)) with (fragattr=s);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create P table failed!");

	sprintf(qry, "INSERT INTO P (d, k, n, s, delta, runtime) VALUES (%d, %d, %u, 0, 0.0, current_timestamp);", d, k, n);
	res = PQexec(conn, qry);
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into P table failed!");
	PGFCM_P(conn);

	sprintf(qry, "SELECT count(*) FROM %s;", SH);
	PRINT("Run `%s'.\n", qry);			
	res = PQexec(conn, qry);	
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Select into n failed!");
	n=atol(PQgetvalue(res, 0, 0));
			
	PRINT("Fill memberships.\n");
	srand(time(NULL));	
	for (icnt=1; icnt<=n; icnt++)
		for (jcnt=1; jcnt<=k; jcnt++) {
			tmp = eps + (float)(rand())/RAND_MAX ;
			sprintf(qry, "INSERT INTO U VALUES(%u, %u, %.10f);", icnt, jcnt, tmp);
			PRINT("Run `%s'.\n", qry);			
			res = PQexec(conn, qry);
			PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Filling in U table failed!");
		}
	PGFCM_U(conn);	
		
	PRINT("Normalization of memberships.\n");	
	res = PQexec(conn, "UPDATE U SET val = val/U1.tmp\
		FROM (SELECT i, sum(val) AS tmp	FROM U GROUP BY i) AS U1\
		WHERE U1.i = U.i;");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Update U table failed!");

#ifdef __NEVER
	fprintf(stderr, "TOO MANY QU HERE");
	for (icnt=1; icnt<=n; icnt++) {
		sprintf(qry, "SELECT sum(val) FROM U WHERE i=%u", icnt);
		PRINT("Run %s.\n", qry);	
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Selection of tuples U table failed!");
		tmp=atof(PQgetvalue(res, 0, 0));
		sprintf(qry, "UPDATE U SET val = val/%.20f WHERE i=%u", tmp, icnt);
		PRINT("Run %s.\n", qry);			
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insertion of tuples into U table failed!");
	}	
#endif
	PGFCM_U(conn);		

	PRINT("Create indices.\n");
	sprintf(qry, "CREATE INDEX SVindexI ON %s (i);", SV);
	res = PQexec(conn, qry);
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SVindexI failed!");
	sprintf(qry, "CREATE INDEX SVindexIL ON %s (i,l);", SV);
	res = PQexec(conn, qry);
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SVindexIL failed!");
	sprintf(qry, "CREATE INDEX SVindexL ON %s (l);", SV);	
	res = PQexec(conn, qry);
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SVindexL failed!");
	res = PQexec(conn, "CREATE INDEX CindexL ON C(l);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create CindexL failed!");	
	res = PQexec(conn, "CREATE INDEX CindexJL ON C(j, l);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create CindexJL failed!");	
	res = PQexec(conn, "CREATE INDEX SDindexI ON SD(i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SDindexI failed!");	
	res = PQexec(conn, "CREATE INDEX SDindexIJ ON SD(i, j);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create SDindexIJ failed!");	
	res = PQexec(conn, "CREATE INDEX UindexIJ ON U(i, j);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create UindexIJ failed!");	
	res = PQexec(conn, "CREATE INDEX UindexI ON U(i);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create UindexI failed!");	
	res = PQexec(conn, "CREATE INDEX UTindexIJ ON UT(i, j);");
	PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Create UTindexIJ failed!");

	steps = 0;
	start = clock();

	while (TRUE) {

		steps++;
		start_iter = clock();

		struct timespec vartime = timer_start();
		
		fprintf(stderr, "Run iteration #%d\n", steps);
		PRINT("Compute centroids.\n");
		res = PQexec(conn, "TRUNCATE C;");
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate C table failed!");
		
		sprintf(qry, "INSERT INTO C\
			SELECT R.j, %s.l, sum(R.s * %s.val)/sum(R.s) AS val\
			FROM (SELECT i, j, U.val^%.20f AS s\
			FROM U) AS R, %s\
			WHERE R.i = %s.i\
			GROUP BY j, l;", 
			SV, SV, m, SV, SV);
		PRINT("Run `%s'.\n", qry);
		res = PQexec(conn, qry);	
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into C table failed!");
		PGFCM_C(conn);

		PRINT("Compute distances.\n");	
		res = PQexec(conn, "TRUNCATE SD;");
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate SD table  failed!");
		sprintf(qry, "INSERT INTO SD\
			SELECT i, j, sum((%s.val - C.val)^2) AS dist\
			FROM %s, C\
			WHERE %s.l = C.l\
			GROUP BY i, j;", 
			SV, SV, SV);
		PRINT("Run `%s'.\n", qry);		
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into SD table  failed!");
		PGFCM_SD(conn);	
	
		PRINT("Compute memberships.\n");	
		res = PQexec(conn, "TRUNCATE UT;");
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate UT table  failed!");	
		sprintf(qry, "INSERT INTO UT\
				SELECT SD.i, j, SD.dist^(2.0/(1.0-%.20f)) * SD1.den AS val\
				FROM (SELECT i, 1.0/sum(dist^(2.0/(1.0-%.20f))) AS den\
					FROM SD GROUP BY i) AS SD1, SD\
				WHERE SD.i = SD1.i;", 
				m, m);
		PRINT("Run `%s'.\n", qry);	
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into UT table failed!");
		PGFCM_UT(conn);	
		PGFCM_U(conn);		
		
		PRINT("Updating phase.\n");	
		#ifdef __NEVER
		sprintf(qry, "create temp table tmp as\
		    select ut.i, ut.val as val1, u.val as val2\
		    from ut,u\
		    where ut.i=u.i and ut.j=u.j;");
		#endif
		sprintf(qry, "SELECT max(abs(UT.val - U.val)) FROM UT, U WHERE UT.i = U.i AND UT.j = U.j;");
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_TUPLES_OK, "Select from UT, U tables failed!");
		PRINT("Convert tmp.\n");			
		tmp=atof(PQgetvalue(res, 0, 0));
	
		PRINT("Update P table.\n");			
		sprintf(qry, "INSERT INTO P VALUES (%d, %d, %u, %u, %.20f, current_timestamp);", d, k, n, steps, tmp);
		res = PQexec(conn, qry);
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into P table failed!");
		PGFCM_P(conn);
	
		PRINT("Update memberships.\n");
		res = PQexec(conn, "TRUNCATE U;");
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Truncate U table failed!");
		res = PQexec(conn, "INSERT INTO U SELECT * FROM UT;");
		PG_ASSERT(conn, res, PQresultStatus(res)==PGRES_COMMAND_OK, "Insert into U table failed!");
		PGFCM_U(conn);

		end_iter = clock();		
		//fprintf(stderr, "Stop iteration #%d (running time is %2.f sec.)\n", steps, (end_iter - start_iter)/CLOCKS_PER_SEC);

		//long time_elapsed_nanos = timer_end(vartime);
		//fprintf(stderr, "Time taken (nanoseconds): %ld\n", time_elapsed_nanos);

		PRINT("Check stopping criterion.\n");			
		if ((tmp < eps) || (steps==itermax)) {
			end = clock();
			return (end - start)/CLOCKS_PER_SEC;
		}
	}
}



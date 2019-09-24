/*
Parallel Dynamic Itemset Counting.
Implementation of the main method.

(c) 2016 Mikhail Zymbler
*/

#include "dic.h"
#include "params.h"
#include "debugger.h"
#include "profiler.h"
#include "system.h"
#include "itemsets.h"
#include "bitmap.h"
#include "omp.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <assert.h>
#include <limits.h>
#ifdef _WIN64
#include <intrin.h> 
#else
#include "immintrin.h"
#endif

using namespace std;

// Global data
vector<struct itemset_t> DASHED;// Dashed set (CIRCLE union BOX, that is suspected infrequent set united with suspected frequent set)
vector<struct itemset_t> SOLID;	// Solid set (CIRCLE union BOX, that is confirmed infrequent set united with confirmed frequent set)
vector<struct itemset_t> CAND;	// Candidate itemsets
dic_stat_t DICSTAT;				// Statistics

double 							// Returns: runtime of the algorithm
dic(							// Input:
								// Bitmap of transaction database (global object BITMAP defined in "bitmap.h")
	unsigned long long int n,	// number of transactions
	unsigned int m,				// number of items
	unsigned long long minsup,	// minimum support count
	unsigned long long M,		// number of transactions to be processed before the stop
	int num_of_threads)			// number of threads to run the algorithm
								// Output: 
								// statistics on execution				
								// frequent itemsets (global object BOX defined in "itemsets.h")
{
	double start = omp_get_wtime();

	long long int max_stopno = (n / M) + (n % M > 0);
	assert(max_stopno >= 1);
	long long int stopno = 0;
	long long int first, last, dashedsize, solidsize, candsize;
	long long int prunedcnt = 0;
	DICSTAT.passes = 0;
	DICSTAT.candtotal = 0;
	DICSTAT.candpruned = 0;

	PRINT("n==%llu\tm==%lu\tM==%llu\tminsup==%llu\n", n, m, M, minsup);
	PRINT_BITMAP("BITMAP is:");

	START("Initialize Solid set for the first pass over transaction database");
	for (item_t i = 0; i < m; i++) {
		itemset_t item;
		item.k = 1;
		item.support = 0;
		item.stopno = 0;
		item.shape = CIRCLE;
		for (unsigned int j = 0; j < TRANSACTION_BITMASK_LEN; j++)
			item.bitmask[j] = 0;
		setbit(item.bitmask[i / 64], i % 64);
		PRINT_ITEMSET(item);
		SOLID.push_back(item);
		DICSTAT.candtotal++;
	}
	PRINT_SOLID_CIRCLE("Solid Circle after initialization");
	FINISH("Initialize Solid set for the first pass over transaction database");

	START("FIRST PASS over transaction database");
	DICSTAT.passes = 1;
	// Support counting
	solidsize = (long long)SOLID.size();
	PRF_START(start1);

	if (solidsize >= num_of_threads) {
		PRINT("Number of candidates in the first pass is greater than threads, parallelizing by candidates\n");
		#pragma omp for schedule(guided,1)
		for (long long i = 0; i < solidsize; i++) {
			unsigned long long support = SOLID[i].support;
			__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
			for (long long j = 0; j < n; j++) {
				unsigned long long match_cnt = 0;
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					PRINT("%llu AND %llu == %llu\n", SOLID[i].bitmask[k], BITMAP[j][k], SOLID[i].bitmask[k] & BITMAP[j][k]);
					match_cnt += ((SOLID[i].bitmask[k] & BITMAP[j][k]) == SOLID[i].bitmask[k]);
					PRINT("match_cnt==%llu\n", match_cnt);
				}
				assert(match_cnt >= 0 && match_cnt <= TRANSACTION_BITMASK_LEN);
				support += (match_cnt == TRANSACTION_BITMASK_LEN);
			}
			SOLID[i].support = support;
			//printf("SOLID[%llu].support==%llu\n", i, SOLID[i].support);
			assert(SOLID[i].support <= n);
		}
	} 
	else {
		PRINT("Number of candidates in the first pass is less than threads, parallelizing by transactions\n");
		omp_set_nested(true);
		#pragma omp parallel for num_threads(num_of_threads == 1 ? 1 : solidsize) schedule(guided,1)
		for (long long i = 0; i < solidsize; i++) {
			unsigned long long support = SOLID[i].support;
			#pragma omp parallel for reduction(+:support) num_threads(num_of_threads == 1 ? 1 : num_of_threads/solidsize) schedule(guided,1)
			for (long long j = 0; j < n; j++) {
				unsigned long long match_cnt = 0;
				__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					PRINT("%llu AND %llu == %llu\n", SOLID[i].bitmask[k], BITMAP[j][k], SOLID[i].bitmask[k] & BITMAP[j][k]);
					match_cnt += ((SOLID[i].bitmask[k] & BITMAP[j][k]) == SOLID[i].bitmask[k]);
					PRINT("match_cnt==%llu\n", match_cnt);
				}
				support += (match_cnt == TRANSACTION_BITMASK_LEN);
		}
		SOLID[i].support = support;
		assert(SOLID[i].support <= n);
	}

#ifdef __NEVER
		#pragma omp parallel for num_threads(num_of_threads) schedule(guided,1)
		for (long long j = 0; j < (long long)n; j++) {
			for (long long i = 0; i < solidsize; i++) {
				unsigned long long match_cnt = 0;
				__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					match_cnt += ((SOLID[i].bitmask[k] & BITMAP[j][k]) == SOLID[i].bitmask[k]);
				}
				assert(match_cnt >= 0 && match_cnt <= TRANSACTION_BITMASK_LEN);
				#pragma omp atomic
				SOLID[i].support += (match_cnt == TRANSACTION_BITMASK_LEN);
				assert(SOLID[i].support <= n);
			}
		}
#endif	
	}

	// Move appropriate itemsets from Solid Circle set to Solid Box set
	#pragma omp parallel for num_threads(num_of_threads) schedule(guided,1)
	for (long long i = 0; i < solidsize; i++) 
		if (SOLID[i].support >= minsup) 
				SOLID[i].shape = BOX;
			
	PRF_FINISH(finish1);

	SOLID.erase(remove_if(SOLID.begin(), SOLID.end(), is_inactive_itemset), SOLID.end());
	PRINT_SOLID_CIRCLE("Solid Circle: after the first pass");
	PRINT_SOLID_BOX("Solid Box: after the first pass");

	// Add 2-candidates to Dashed Circle set
	#pragma omp parallel for num_threads(num_of_threads) schedule(guided,1)
	for (long long i = 0; i < solidsize; i++) {
		itemset_t I = SOLID[i];
		for (long long j = i + 1; j < solidsize; j++) {
			itemset_t J = SOLID[j];
			if (I.shape == BOX && J.shape == BOX) {
				itemset_t cand;
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					cand.bitmask[k] = I.bitmask[k] | J.bitmask[k];
				}
				cand.k = 2;
				cand.support = 0;
				cand.stopno = 0;
				cand.shape = CIRCLE;
				#pragma omp critical
				if (find(DASHED.begin(), DASHED.end(), cand) == DASHED.end()) {
					DASHED.push_back(cand);
					PRINT("Candidate added: ");
					PRINT_ITEMSET(cand);
					DICSTAT.candtotal++;
				} // else 
					// PRINT("Candidate not added (already exists)\n");
			}
		}
	}
	PRINT_DASHED_CIRCLE("Dashed Circle: after the first pass");
	FINISH("FIRST PASS over transaction database");

	START("Main loop");
#pragma omp parallel num_threads(num_of_threads) shared(n, m, DASHED, dashedsize, SOLID, solidsize, CAND, candsize, stopno, first, last, prunedcnt, DICSTAT)
{
	while (!DASHED.empty()) {
	#pragma omp single
	{
		START("01 omp single");
		stopno++;
		//passes = ((double) stopno) / max_stopno;
		if (stopno > max_stopno) {
			stopno = 1;
			//passes++;
		}
		first = (stopno - 1) * M; 
		assert(first >= 0 && first < (long long)n);
		last = stopno * M;
		assert(last > first && last <= (long long)n);
		#pragma omp atomic
		DICSTAT.passes += (((double)M) / n);
		dashedsize = (long long)DASHED.size();
		FINISH("01 omp single");
	}
		START("Process %llu--%llu transactions (passes==%.2f)", first + 1, last, DICSTAT.passes);
		START("Count support (stopno==%llu\tfirst==%llu\tlast==%llu)", stopno, first + 1, last);						
		PRF_START(start2);
		// Support counting. Check a transaction against itemsets from Dashed Circle set and Dashed Box set
		//printf("passes==%.2f\tBefore support counting: DASHED.size()==%llu\n", passes, dashedsize);
		#pragma omp for schedule(guided,1)
		for (long long i = 0; i < dashedsize; i++) {
			DASHED[i].stopno++;	
		}

#define __PARALLELIZE_BY_BOTH

#ifdef __PARALLELIZE_BY_BOTH
		if (dashedsize >= num_of_threads) {
			PRINT("Number of candidates is greater than threads, parallelizing by candidates\n");
			#pragma omp for schedule(guided,1)
			for (long long i = 0; i < dashedsize; i++) {
				unsigned long long support = DASHED[i].support;
				__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
				for (long long j = first; j < last; j++) {
					unsigned long long match_cnt = 0;
					for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
						PRINT("%llu AND %llu == %llu\n", DASHED[i].bitmask[k], BITMAP[j][k], DASHED[i].bitmask[k] & BITMAP[j][k]);
						match_cnt += ((DASHED[i].bitmask[k] & BITMAP[j][k]) == DASHED[i].bitmask[k]);
						PRINT("match_cnt==%llu\n", match_cnt);
					}
					assert(match_cnt >= 0 && match_cnt <= TRANSACTION_BITMASK_LEN);
					support += (match_cnt == TRANSACTION_BITMASK_LEN);
				}
				DASHED[i].support = support;
				//printf("DASHED[%llu].support==%llu\n", i, DASHED[i].support);
				assert(DASHED[i].support <= n);
			}
		}
		else {
			PRINT("Number of candidates is less than threads, parallelizing by transactions\n");

#ifdef __NEVER			
			for (long long i = 0; i < dashedsize; i++) {
				unsigned long long support = DASHED[i].support;
				#pragma omp parallel for reduction(+:support) num_threads(num_of_threads == 1 ? 1 : num_of_threads/dashedsize) schedule(guided,1)
				for (long long j = first; j < last; j++) {
					unsigned long long match_cnt = 0;
					__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
					for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
						PRINT("%llu AND %llu == %llu\n", DASHED[i].bitmask[k], BITMAP[j][k], DASHED[i].bitmask[k] & BITMAP[j][k]);
						match_cnt += ((DASHED[i].bitmask[k] & BITMAP[j][k]) == DASHED[i].bitmask[k]);
						PRINT("match_cnt==%llu\n", match_cnt);
					}
					support += (match_cnt == TRANSACTION_BITMASK_LEN);
				}
				DASHED[i].support = support;
				assert(DASHED[i].support <= n);
			}
#endif
			#pragma omp for schedule(guided,1)
			for (long long j = first; j < last; j++) {
				for (long long i = 0; i < dashedsize; i++) {
					unsigned long long match_cnt = 0;
					__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
					for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
						PRINT("%llu AND %llu == %llu\n", DASHED[i].bitmask[k], BITMAP[j][k], DASHED[i].bitmask[k] & BITMAP[j][k]);
						match_cnt += ((DASHED[i].bitmask[k] & BITMAP[j][k]) == DASHED[i].bitmask[k]);
						PRINT("match_cnt==%llu\n", match_cnt);
					}
					assert(match_cnt >=0 && match_cnt <= TRANSACTION_BITMASK_LEN);
					#pragma omp atomic
					DASHED[i].support += (match_cnt == TRANSACTION_BITMASK_LEN);
					assert(DASHED[i].support <= n);
				}
			}
		}
#endif
#ifdef __PARALLELIZE_BY_CANDIDATES
		#pragma omp for schedule(guided,1)
		for (long long i = 0; i < dashedsize; i++) {
			unsigned long long support = DASHED[i].support;
			__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
			for (long long j = first; j < last; j++) {
				unsigned long long match_cnt = 0;
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					PRINT("%llu AND %llu == %llu\n", DASHED[i].bitmask[k], BITMAP[j][k], DASHED[i].bitmask[k] & BITMAP[j][k]);
					match_cnt += ((DASHED[i].bitmask[k] & BITMAP[j][k]) == DASHED[i].bitmask[k]);
					PRINT("match_cnt==%llu\n", match_cnt);
				}
				assert(match_cnt >= 0 && match_cnt <= TRANSACTION_BITMASK_LEN);
				support += (match_cnt == TRANSACTION_BITMASK_LEN);
			}
			DASHED[i].support = support;
			//printf("DASHED[%llu].support==%llu\n", i, DASHED[i].support);
			assert(DASHED[i].support <= n);
		}			
#endif
#ifdef __PARALLELIZE_BY_TRANSACTIONS
		#pragma omp for schedule(guided,1)
		for (long long j = first; j < last; j++) {
			for (long long i = 0; i < dashedsize; i++) {
				unsigned long long match_cnt = 0;
				__ASSUME_ALIGNED(BITMAP, ALIGN_SIZE);
				for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
					PRINT("%llu AND %llu == %llu\n", DASHED[i].bitmask[k], BITMAP[j][k], DASHED[i].bitmask[k] & BITMAP[j][k]);
					match_cnt += ((DASHED[i].bitmask[k] & BITMAP[j][k]) == DASHED[i].bitmask[k]);
					PRINT("match_cnt==%llu\n", match_cnt);
				}
				assert(match_cnt >=0 && match_cnt <= TRANSACTION_BITMASK_LEN);
				#pragma omp atomic
				DASHED[i].support += (match_cnt == TRANSACTION_BITMASK_LEN);
				assert(DASHED[i].support <= n);
			}
		}
#endif
		FINISH("Count support (stopno==%llu\tfirst==%llu\tlast==%llu)", stopno, first + 1, last);
		PRF_FINISH(finish2);
		PRINT_DASHED_CIRCLE("Dashed Circle after counting support");
		PRINT_DASHED_BOX("Dashed Box after counting support");

		// Move appropriate itemsets from Dashed Circle set to Dashed Box set 
		PRF_START(start3);
	#pragma omp single
	{
		START("02 omp single");
		dashedsize = (long long)DASHED.size();
		prunedcnt = 0;
		FINISH("02 omp single");
	}
		#pragma omp for schedule(guided, 1) // reduction(+:prunedcnt) 
		for (long long i = 0; i < dashedsize; i++) {
			itemset_t I = DASHED[i];			
			if (I.shape == CIRCLE) {
				if (I.support >= minsup) {
					DASHED[i].shape = BOX;
					PRINT("This itemset changed its shape: ");
					PRINT_ITEMSET(DASHED[i]);								
				} 
				else 
					// Check possibility of pruning for candidate itemset
					if (I.support + M * (max_stopno - I.stopno) < minsup) {
						// Prune this candidate itemset
						DASHED[i].shape = NIL;
						#pragma omp atomic
						prunedcnt++;
						//printf("... pruned as clearly infrequent!\n");
						PRINT("This itemset is pruned (its possible highest support is %llu, less than minsup==%llu): ", DASHED[i].support + M * (max_stopno - DASHED[i].stopno), minsup);
						PRINT_ITEMSET(DASHED[i]);
						#pragma omp atomic
						DICSTAT.candpruned++;
					}
			}		
		}				
#define NPRUNE_SUPERSETS
#ifdef PRUNE_SUPERSETS
#pragma omp single
{		
		if (prunedcnt > 0) {
			// Try to prune supersets of clearly infrequent candidate itemsets
			for (long long i = 0; i < dashedsize && prunedcnt > 0; i++) {
				itemset_t I = DASHED[i];
				if (I.shape == NIL) {
					prunedcnt--;
					for (long long j = 0; j < dashedsize; j++) {
				    	itemset_t J = DASHED[j];
						unsigned long long match_cnt = 0;
						if ((I.k < J.k) && (J.shape == CIRCLE)) {
							for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
								PRINT("%llu AND %llu == %llu\n", I.bitmask[k], J.bitmask[k], I.bitmask[k] & J.bitmask[k]);
								if ((I.bitmask[k] & J.bitmask[k]) == I.bitmask[k]) {
									PRINT("This itemset is pruned as superset of pruned candidate: ");
									PRINT_ITEMSET(J);
									J.shape = NIL;
									DICSTAT.candpruned++;
								}
							}
						}
					}
				}
			}
		}
}
#endif				
		// Remove all the pruned itemsets
		#pragma omp single
		{
		START("03 omp single");
		DASHED.erase(remove_if(DASHED.begin(), DASHED.end(), is_inactive_itemset), DASHED.end());
		FINISH("03 omp single");
		}

		PRF_FINISH(finish3);
		
		// Generate candidate itemsets to be inserted in Dashed Circle set		
		PRF_START(start4);
		#pragma omp single
		{
		START("04 omp single");
		dashedsize = (long long)DASHED.size();
		solidsize = (long long)SOLID.size();
		FINISH("04 omp single");
		}

		#pragma omp for schedule(guided, 1)
		for (long long i = 0; i < dashedsize; i++) {
			itemset_t I = DASHED[i];
			for (long long j = i + 1; j < dashedsize; j++) {
				itemset_t J = DASHED[j];
				if (I.shape == BOX && J.shape == BOX && I.k == J.k) {
					PRINT("1st itemset to join, DB[%llu]: ", i);
					PRINT_ITEMSET(I);
					PRINT("2nd itemset to join, DB[%llu]: ", j);					
					PRINT_ITEMSET(DASHED[j]);
					unsigned long long match_cnt = 0;
					for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
						match_cnt += (I.bitmask[k] == J.bitmask[k]);
					}
					if (match_cnt == TRANSACTION_BITMASK_LEN - 1) {
						unsigned long long mismatchno = ULLONG_MAX;
						for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
							if (I.bitmask[k] != J.bitmask[k]) {
								mismatchno = k;
								break;
							}
						}
						assert(mismatchno >= 0 && mismatchno < TRANSACTION_BITMASK_LEN);
						if (joinable(I.bitmask[mismatchno], J.bitmask[mismatchno])) {
							itemset_t cand;
							for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
								cand.bitmask[k] = I.bitmask[k] | J.bitmask[k];
							}
							cand.k = I.k + 1;
							cand.support = 0;
							cand.stopno = 0;
							cand.shape = CIRCLE;
							#pragma omp critical
							if (find(CAND.begin(), CAND.end(), cand) == CAND.end() && find(DASHED.begin(), DASHED.end(), cand) == DASHED.end()) {
								CAND.push_back(cand);
								PRINT("Candidate added: ");
								PRINT_ITEMSET(cand);
								DICSTAT.candtotal++;
							} // else 
								// PRINT("Candidate not added (already exists)\n");
						} // else
							// PRINT("... not joinable\n");
					} // else
					    // PRINT("... not joinable\n");
				}
			}
			for (long long j = 0; j < solidsize; j++) {
				itemset_t J = SOLID[j];
				if (I.shape == BOX && J.shape == BOX && I.k == J.k) {
					PRINT("1st itemset to join, DB[%llu]: ", i);
					PRINT_ITEMSET(I);
					PRINT("2nd itemset to join, SB[%llu]: ", j);					
					PRINT_ITEMSET(SOLID[j]);
					unsigned long long match_cnt = 0;
					for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
						match_cnt += (I.bitmask[k] == J.bitmask[k]);
					}
					if (match_cnt == TRANSACTION_BITMASK_LEN - 1) {
						unsigned long long mismatchno = ULLONG_MAX;
						for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
							if (I.bitmask[k] != J.bitmask[k]) {
								mismatchno = k;
								break;
							}
						}
						assert(mismatchno >= 0 && mismatchno < TRANSACTION_BITMASK_LEN);
						if (joinable(I.bitmask[mismatchno], J.bitmask[mismatchno])) {
							itemset_t cand;
							for (long long k = 0; k < TRANSACTION_BITMASK_LEN; k++) {
								cand.bitmask[k] = I.bitmask[k] | J.bitmask[k];
							}
							cand.k = I.k + 1;
							cand.support = 0;
							cand.stopno = 0;
							cand.shape = CIRCLE;
							#pragma omp critical
							if ((find(CAND.begin(), CAND.end(), cand) == CAND.end()) && (find(DASHED.begin(), DASHED.end(), cand) == DASHED.end())) {
								CAND.push_back(cand);
								PRINT("Candidate added: ");
								PRINT_ITEMSET(cand);
								DICSTAT.candtotal++;
							} // else 
								// PRINT("Candidate not added (already exists)\n");
						} // else
							// PRINT("... not joinable\n");
					}
					// else
							// PRINT("... not joinable\n");
				}
			}
		}
		PRF_FINISH(finish4);

		PRINT_DASHED_CIRCLE("Dashed Circle (after removing of itemsets to be inserted in Dashed Box)");		
		PRINT_DASHED_BOX("Dashed Box (after insertion of itemsets from Dashed Circle)");
		PRINT_CANDIDATES("Candidates to be inserted in Dashed Circle (marked as CIRCLEs)");
		PRF_START(start5);
		PRINT("Insert candidates");
	#pragma omp single
	{
		START("05 omp single");
		candsize = (long long)CAND.size();
		for (long long i = 0; i < candsize; i++) {
			DASHED.push_back(CAND[i]);
			//CAND[i].shape = NIL;
		}
	}
		PRINT("Clear candidates");
	#pragma omp single
	{		
		CAND.clear();
		//CAND.erase(remove_if(CAND.begin(), CAND.end(), is_inactive_itemset), CAND.end());
		dashedsize = (long long)DASHED.size();
		FINISH("05 omp single");
	}
	
		PRINT_DASHED_CIRCLE("Dashed Circle after insertion of candidate itemsets");
		PRF_FINISH(finish5);
		START("Check full pass completion (stopno==%llu\tfirst==%llu\tlast==%llu)", stopno, first + 1, last);
		PRF_START(start6);
		//printf("passes==%.2f\tAfter adding candidates: DASHED.size()==%llu\n", passes, dashedsize);		
		#pragma omp for schedule(guided, 1)
		for (long long i = 0; i < dashedsize; i++) {
			itemset_t I = DASHED[i];
			if (I.stopno == max_stopno) {
				if (I.support >= minsup) {
					I.shape = BOX;
					#pragma omp critical
					SOLID.push_back(I);
				}
				DASHED[i].shape = NIL;
			}
		}
		#pragma omp single
		{
		START("06 omp single");
		DASHED.erase(remove_if(DASHED.begin(), DASHED.end(), is_inactive_itemset), DASHED.end());
		FINISH("06 omp single");
		}
		PRF_FINISH(finish6);
		PRINT_DASHED_CIRCLE("Dashed Circle: after removing itemsets with full pass completion");
		PRINT_SOLID_CIRCLE("Solid Circle: after check full pass completion");
		PRINT_SOLID_BOX("Solid Box: after check full pass completion");
		FINISH("Check full pass completion (stopno==%llu\tfirst==%llu\tlast==%llu)", stopno, first + 1, last);

		FINISH("Process %llu--%llu transactions (passes==%.2f)", first + 1, last, DICSTAT.passes);
	} // of "while (!DASHED.empty())"
}
	FINISH("Main loop");
	PRINT_SOLID_BOX("Solid Box: final contents");

	double stop = omp_get_wtime();

	return stop - start;
}


// Print SOLID BOXes
// Input: n -- number of transactions
void print_solid(unsigned long long int n)
{
	unsigned long long size = 0;
	for (unsigned long i = 0; i < SOLID.size(); i++) {
		if (SOLID[i].shape == BOX) {
			size++;
			printf("%llu:\t|", size);
			for (unsigned long long idx = 0; idx < TRANSACTION_BITMASK_LEN; idx++) {
				for (unsigned int b = 0; b < 64; b++) {
					if (isbitset(SOLID[i].bitmask[idx], b)) {
						printf("%llu|", idx * 64 + b + 1); 
					} 
				} 
			} 
			printf("\tsupp==%llu (%.2f)\n", SOLID[i].support, ((double)SOLID[i].support) / n);
		}
	}
}

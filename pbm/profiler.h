/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Profiler.
(c) Mikhail Zymbler 
*/

#ifndef PROFILER_H
#define PROFILER_H

#include <omp.h>

#define PROFILE

extern double start1, finish1, start2, finish2, start3, finish3, start4, finish4, start5, finish5;//, start6, finish6, start7, finish7, start8, finish8, start9, finish9, start10, finish10;

// Place START and FINISH tags
#ifdef NPROFILE
#define PRF_START(name)
#define PRF_FINISH(name)
#else
#define PRF_THREAD	(0)
#define PRF_START(startpoint) do { if (omp_get_thread_num() == PRF_THREAD) startpoint += omp_get_wtime(); } while (0);
#define PRF_FINISH(finishpoint) do { if (omp_get_thread_num() == PRF_THREAD) finishpoint += omp_get_wtime(); } while (0);
#endif

// Init profile tags
#ifdef NPROFILE
#define PRF_INIT
#else
#define PRF_INIT do { start1=finish1=start2=finish2=start3=finish3=start4=finish4=start5=finish5=0.0; } while (0);//=start6=finish6=start7=finish7=start8=finish8=start9=finish9=start10=finish10=0.0; } while (0);
#endif

#endif

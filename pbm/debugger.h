/* 
Parallel algorithm for time series subsequence similarity search under DTW measure.
Simple debugger.
(c) Mikhail Zymbler 
*/

#ifndef DEBUGGER_H
#define DEBUGGER_H

#define DEBUG

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <omp.h>

// Output a message
#ifdef NDEBUG
#define DBG(msg, ...)
#define PRINT(msg, ...)
#else
#ifdef _WIN64
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define DBG_THREAD	(0)
#define DBG(msg, ...) do { if (omp_get_thread_num() == 0) { printf("DEBUG %s:%d " msg "\n", __FILENAME__, __LINE__, ##__VA_ARGS__); fflush(stdout); } } while (0); 
#define PRINT(msg, ...) do { if (omp_get_thread_num() == DBG_THREAD) { printf(msg, ##__VA_ARGS__); fflush(stdout); } } while (0);
#endif

// Output START and FINISH messages
#ifdef NDEBUG
#define START(name, ...)
#define FINISH(name, ...)
#else
#define START(name, ...) PRINT("\nSTART:\t" name "\n", ##__VA_ARGS__)
#define FINISH(name, ...) PRINT("FINISH:\t" name "\n", ##__VA_ARGS__)
#endif

#endif


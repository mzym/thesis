#!/bin/sh
set -ex
source ../config.sh
include="$prefix/include"
lib="$prefix/lib"
for t in butterfly pgfcm; do
	gcc -o test-$t -I$include -L$lib test-$t.c pgfcm.c pgfcm_debug.c csvparser.c -Wl,-Bstatic -lpq -Wl,-Bdynamic -lrt #-fopenmp
done

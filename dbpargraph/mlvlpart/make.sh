#!/bin/sh
set -ex
cd `dirname "$0"`
source ../config.sh
include="$prefix/include"
lib="$prefix/lib"
utilities=(
	export2csv
	import_from_csv
	partitions_from_csv
	coarsen
	uncoarsen
)
	
for t in ${utilities[*]}; do
	gcc -o $t -g -I$include -L$lib src/$t.c -Wl,-Bstatic -lpq -Wl,-Bdynamic -lpthread
done

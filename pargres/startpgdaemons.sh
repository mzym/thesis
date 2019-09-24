#!/bin/sh
set -e
cd `dirname "$0"`
source ./config.sh
./kill-parg.sh
for node in ${nodes[*]}; do
	if ssh $node "export PATH=\"$prefix/bin\"; export LD_LIBRARY_PATH=\"$prefix/lib\"; pg_ctl -D \"$datadir\" start -l \"$datadir/parg.log\" -o \"-i --enable-pargresql=false\""; then
		echo "A daemon started on node '$node'"
	else
		echo "The daemon wouldn't start on node '$node', read '$node:$datadir/parg.log' for details."
		exit 1
	fi
done
./runinis.sh

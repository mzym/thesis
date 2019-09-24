#!/bin/bash
set -e
cd `dirname "$0"`
source ./config.sh
for node in ${nodes[*]:0:n}; do
	if ssh $node "export PATH=\"$prefix/bin\"; export LD_LIBRARY_PATH=\"$prefix/lib\"; pg_ctl -D \"$datadir\" stop -t 3"; then
		echo "A daemon stopped ($node)"
	fi
done
./kill-parg.sh

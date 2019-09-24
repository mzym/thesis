#!/bin/bash
set -ex
source ../config.sh

# Initialize the database
for node in ${nodes[*]:0:n}; do
	echo "node $node"
	ssh "$node" "rm -rf \"$datadir\"; \
		\"$prefix/bin/initdb\" -D \"$datadir\" -U pangres && \
		echo 'host all all 0.0.0.0/0 trust' >> \"$datadir/pg_hba.conf\""
done

echo "importing done"

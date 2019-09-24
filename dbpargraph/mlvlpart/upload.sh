#!/bin/bash
set -e
cd `dirname "$0"`
source ../config.sh

# Initialize the database
for node in ${nodes[*]}; do
	echo "node $node"
	ssh "$node" "rm -rf \"$datadir\"; \
		\"$prefix/bin/initdb\" -D \"$datadir\" -U $USER && \
		echo 'host all all 0.0.0.0/0 trust' >> \"$datadir/pg_hba.conf\""
	echo "node $node, applying the schema"
	echo 'create language plpgsql;' | ssh "$node" 'cat > /tmp/schema.sql'
	cat coarsen.sql uncoarsen.sql | ssh "$node" 'cat >> /tmp/schema.sql'
	ssh "$node" "\"$prefix/bin/postgres\" --single postgres -D \"$datadir\" -j < /tmp/schema.sql &> /tmp/query.log"
done

echo "importing done"

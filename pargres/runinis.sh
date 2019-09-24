#!/bin/bash
set -e
source ./config.sh

nodelist=$(IFS=,; echo "${nodes[*]:0:n}") # join the node list on comma into a string

echo "Starting message passing subsystem on $nodelist"
nohup mpirun -H $nodelist "$prefix/bin/par_inis_daemon" < /dev/null &> ./inis.log &
#mpirun -H $nodelist "$prefix/bin/par_inis_daemon"
echo "Message passing subsystem started."

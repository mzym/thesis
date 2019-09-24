#!/bin/bash
set -ex
cd `dirname "$0"`
source ../config.sh

truncate -s 0 par_libpq.conf
# Initialize the database
if which resolve; then
	resolve="resolve -s"
elif which dig; then
	resolve="dig +short"
fi
for node in ${nodes[*]}; do
	echo "dbname=postgres hostaddr=$($resolve $node) port=5432" >> par_libpq.conf
done

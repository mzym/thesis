#!/bin/bash
set -e
cd `dirname "$0"`
source ../config.sh

hostwrapper() {
	host "$1" | grep -Po 'has address \d+\.\d+\.\d+\.\d+' | grep -Po '\d+\.\d+\.\d+\.\d+'
}

truncate -s 0 par_libpq.conf
# Initialize the database
if which resolve &>/dev/null; then
	resolve="resolve -s"
elif which host &>/dev/null; then
	resolve="hostwrapper"
fi
for node in ${nodes[*]:0:n}; do
	echo "dbname=postgres hostaddr=$($resolve $node) port=5432" >> par_libpq.conf
done

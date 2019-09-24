#!/bin/bash
set -e
cd `dirname "$0"`
source ../config.sh

./query $((RANDOM % n))
echo "Done."

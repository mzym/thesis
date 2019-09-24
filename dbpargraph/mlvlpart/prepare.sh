#!/bin/bash
set -ex
cd `dirname "$0"`
source ../config.sh

./make.sh
./makeconf.sh
./upload.sh

echo "Test GraphPart is ready to run."

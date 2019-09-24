#!/bin/sh
../allocnodes.sh 2 
./makeconf.sh
./make.sh
#./upload-pgfcm.sh
./upload.sh
../startdaemons.sh
set -e
for tst in butterfly; do
	for run in 1; do
		echo "test-$tst run $run"
		/usr/bin/time -f '%e %C' -a -o time.log ./test-$tst > /dev/null
		echo "waiting 10 seconds"
		sleep 10
	done
done

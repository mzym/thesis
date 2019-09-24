#!/bin/sh
../allocnodes.sh 2
./makeconf.sh
./make.sh
#./upload-pgfcm.sh
./upload.sh
../startdaemons.sh
set -e
for tst in pgfcm; do
	for run in 1; do
		echo "test-$tst run $run"
		# SUSY dataset, Tornado-B (8 nodes)
		#/usr/bin/time -f '%e %C' -a -o time.log ./test-$tst 6 2 0.00000000005 1000 SUSY_SH_TABLE_50000.csv SUSY_SV_TABLE_50000.csv > /dev/null
		# HIGGS dataset, Tornado-B (8 nodes)
		#/usr/bin/time -f '%e %C' -a -o time.log ./test-$tst 2 2 0.00000000005 1000 HIGGS_SH_TABLE.csv HIGGS_SV_TABLE.csv > /dev/null
		# SUSY dataset, Tornado-A (2 nodes)
		/usr/bin/time -f '%e %C' -a -o time.log ./test-$tst 6 2 0.00000000005 4 SUSY_SH_TABLE_1000.csv SUSY_SV_TABLE_1000.csv > /dev/null
		# HIGGS dataset, Tornado-A (2 nodes)
		#/usr/bin/time -f '%e %C' -a -o time.log ./test-$tst 2 2 0.00000000005 1000 HIGGS_SH_TABLE.csv HIGGS_SV_TABLE.csv > /dev/null
		echo "waiting 10 seconds"
		sleep 10
	done
done

# This is the configuration file for all the testing scripts.

nodes=(
## strange configuration:
#node101.tornado
#node102.tornado
#node103.tornado node104.tornado
#node199.tornado node200.tornado node201.tornado node202.tornado
#node203.tornado node204.tornado node205.tornado node206.tornado node285 node286.tornado node287.tornado node288.tornado
#node352 node353.tornado node354.tornado node355.tornado node390 node391.tornado node392.tornado node393.tornado node394.tornado node396 node397.tornado node441 node442.tornado node443.tornado node444.tornado node445.tornado

## saturday on omega seminar config
#node255.tornado
#node256.tornado
#node257.tornado node258.tornado
#node259.tornado node260.tornado node261.tornado node262.tornado
#node348.tornado node349.tornado node350.tornado node351.tornado node352.tornado node353.tornado node354.tornado node355.tornado
#node385.tornado node428.tornado node429.tornado node430.tornado node431.tornado node433.tornado node434.tornado node435.tornado node436.tornado node437.tornado node438.tornado node441.tornado node442.tornado node443.tornado node444.tornado node445.tornado

## monday
#node089.tornado
#node090.tornado
#node091.tornado node092.tornado
#node093.tornado node094.tornado node224.tornado node225.tornado
#node226.tornado node227.tornado node228.tornado node229.tornado node270.tornado node271.tornado node366.tornado node367.tornado
#node368.tornado node369.tornado node370.tornado node371.tornado node433.tornado node434.tornado node435.tornado node436.tornado node437.tornado node438.tornado node439.tornado node441.tornado node442.tornado node443.tornado node444.tornado node445.tornado

## tuesday
node027.tornado
node028.tornado
node029.tornado node030.tornado
node031.tornado node032.tornado node033.tornado node348.tornado
node349.tornado node350.tornado node351.tornado node352.tornado node353.tornado node354.tornado node355.tornado node432.tornado
node433.tornado node434.tornado node435.tornado node436.tornado node437.tornado node438.tornado node439.tornado node468.tornado node469.tornado node470.tornado node471.tornado node472.tornado node473.tornado node474.tornado node475.tornado node476.tornado
)
n=${n:=${#nodes[*]}}
echo $n

prefix="$HOME/pargresql-build"
datadir="/tmp/data-parg"

#!/bin/sh

all_pids=`ls /proc | grep -E '^[0-9]+$'`
PID_PSS_RES=""
TOTAL=""

for n in $all_pids; do 
	PID_PSS_RES=`pmap -x $n | tail -1 | awk -F' ' '{print $3}'`" $PID_PSS_RES";
done

for n in $PID_PSS_RES; do
	TOTAL=$((n+TOTAL))
done
echo "Your user space memory consumption size(with Proportional shared lib) is $TOTAL" KBytes

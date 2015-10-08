#!/bin/bash -

if [ $# -eq 0 ]; then
	all_ipc=`ipcs -m | grep 0x | awk '{print $1}'`
else
	all_ipc=$1
fi

for ipc in $all_ipc
do
	ipcrm -M $ipc
done

# 再次打印ipcs，验证是否已经删除干净
ipcs -m

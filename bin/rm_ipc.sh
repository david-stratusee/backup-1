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

# �ٴδ�ӡipcs����֤�Ƿ��Ѿ�ɾ���ɾ�
ipcs -m

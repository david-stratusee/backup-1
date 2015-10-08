#!/bin/bash -
#===============================================================================
#          FILE: test_perf.sh
#         USAGE: ./test_perf.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月12日 10:45
#      REVISION:  ---
#===============================================================================

ulimit -c unlimited

args="-a 4 -b 1 -t 300"
#list="ds_512.txt ds_100k.txt ds_1k.txt ds_10k.txt"
#list="512 1k 10k 100k"
list="100k"
userstr=$1

if [ ! -d ../result ]; then
	mkdir ../result/
fi
sudo rm -f ../result/5min_${userstr}.log
sudo rm -f ../result/5min_${userstr}.csv

for file in $list; do
    ../multi_test -f ../data/ds_${file}.txt -d ${userstr} -o ../result/5min_${userstr}.csv ${args} >>../result/5min_${userstr}.log 2>&1
    sleep 60

    ../multi_test -f ../data/ds_${file}.txt -d s_${userstr} -o ../result/5min_${userstr}.csv -s ${args} >>../result/5min_${userstr}.log 2>&1
    sleep 60
done

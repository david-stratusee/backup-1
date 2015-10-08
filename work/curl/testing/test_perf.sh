#!/bin/bash -
#===============================================================================
#          FILE: test_perf.sh
#         USAGE: ./test_perf.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月12日 10:45
#      REVISION:  ---
#===============================================================================

set${userstr} -o nounset                              # Treat unset variables as an error
ulimit -c unlimited

args="-a 40 -n 8 -b 8 -t 600"
#list="ds_512.txt ds_100k.txt ds_1k.txt ds_10k.txt"
list="512 100k 1k 10k"
if [ $# -gt 0 ]; then
    userstr=_$1
else
    userstr=""
fi

./add_route_2.sh del
for file in $list; do
    ../multi_test -f ../data/ds_${file}.txt -d direct${userstr} -o ../result/aie_10min${userstr}.csv ${args} >>../result/aie_10min${userstr}.log 2>&1
    sleep 60

    ./add_route_3.sh
    ../multi_test -f ../data/ds_${file}.txt -d 2c${userstr} -o ../result/aie_10min${userstr}.csv ${args} >>../result/aie_10min${userstr}.log 2>&1
    ./add_route_3.sh del
    sleep 60

    ./add_route_2.sh
    ../multi_test -f ../data/ds_${file}.txt -d 1c${userstr} -o ../result/aie_10min${userstr}.csv ${args} >>../result/aie_10min${userstr}.log 2>&1
    ./add_route_2.sh del
    sleep 60

    ../multi_test -f ../data/ds_${file}.txt -d s_direct${userstr} -o ../result/aie_10min${userstr}.csv -s ${args} >>../result/aie_10min${userstr}.log 2>&1
    sleep 60

    ./add_route_3.sh
    ../multi_test -f ../data/ds_${file}.txt -d s_2c${userstr} -o ../result/aie_10min${userstr}.csv -s ${args} >>../result/aie_10min${userstr}.log 2>&1
    ./add_route_3.sh del
    sleep 60

    ./add_route_2.sh
    ../multi_test -f ../data/ds_${file}.txt -d s_1c${userstr} -o ../result/aie_10min${userstr}.csv -s ${args} >>../result/aie_10min${userstr}.log 2>&1
    ./add_route_2.sh del
    sleep 60
done


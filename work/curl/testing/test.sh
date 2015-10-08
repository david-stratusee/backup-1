#!/bin/bash -
#===============================================================================
#          FILE: test.sh
#         USAGE: ./test.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年01月18日 07:47
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#ulimit -n 19999
#echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle
#echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse

#desc=$1
#output=$2
req=100000
agent=400
rampup=10
during=60

ulimit -c unlimited

list="ds_512.txt ds_100k.txt ds_1k.txt ds_10k.txt"

echo ./multi_test -f data/ds_512.txt -a 400 -t 10 -d "preview"
../multi_test -f ../data/ds_512.txt -a 400 -t 10 -d "preview"

for file in $list; do
    sleep 10
    #echo ./multi_test -a ${agent} -f data/${file} -d \"${desc}\" -o ${output} -r ${rampup} -t ${during}
    #./multi_test -a ${agent} -f data/${file} -d "${desc}" -o ${output} -r ${rampup} -t ${during}
    echo ./multi_test -f data/${file} $@
    ../multi_test -f ../data/${file} $@
done


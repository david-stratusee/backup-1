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

ulimit -n 19999
echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
#python run.py -r 10 -a 80 -x cms.xml

list="512 1k 10k"
for file in $list; do
    sudo python run.py -r 0 -a 4 -m -d 60 -o result -n ${file} -x ./xml/ds2_${file}.xml >>result/1min${userstr}.log 2>&1
    sleep 30

    sudo python run.py -r 0 -a 4 -m -d 60 -o result -n ${file} -x ./xml/ds2_${file}_s.xml >>result/1min${userstr}.log 2>&1
    sleep 30
done

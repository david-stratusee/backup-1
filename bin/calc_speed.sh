#!/bin/bash -
#===============================================================================
#          FILE: calc_speed.sh
#         USAGE: ./calc_speed.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/10/01 17:19
#===============================================================================

set -o nounset                              # Treat unset variables as an error

if [ $# -eq 0 ]; then
    echo "One argument is needed"
    exit 0
fi

netstat -anb | head -2

line=`netstat -anb | grep ESTABLISHED | grep "$1"`
recv_bytes=`echo $line | awk '{print $7}'`
send_bytes=`echo $line | awk '{print $8}'`
echo $line

sleep 1
line=`netstat -anb | grep ESTABLISHED | grep "$1"`
recv_bytes_2=`echo $line | awk '{print $7}'`
send_bytes_2=`echo $line | awk '{print $8}'`
echo $line

let recv_diff=recv_bytes_2-recv_bytes
let send_diff=send_bytes_2-send_bytes

echo ------------------

let recv_k=recv_diff/1024
let send_k=send_diff/1024
echo -e "Recv: $recv_diff\t${recv_k}KB"
echo -e "Send: $send_diff\t${send_k}KB"

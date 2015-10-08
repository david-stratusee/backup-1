#!/bin/bash -
#===============================================================================
#          FILE: get_box.sh
#         USAGE: ./get_box.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/07/23 09:14
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

box_port=9090
#box_port=8099

dstip_file="/tmp/dstip_file"
get_dstip=0
if [ -f ${dstip_file} ]; then
        dstip=`cat ${dstip_file}`
        if [ "$dstip" != "" ]; then
                echo nmap -sS -n -p ${box_port} -oG - ${dstip}
                count=`sudo nmap -sS -n -p ${box_port} -oG - ${dstip} 2>/dev/null | grep -c open`
                if [ $count -gt 0 ]; then
                        get_dstip=1
                fi
        fi
fi

if [ $get_dstip -eq 0 ]; then
        >$dstip_file
        localip=`ifconfig | grep -A 3 "en0:" | grep "inet " | awk '{print $2}'`
        echo nmap -sS -n -p ${box_port} -oG - ${localip}/24
        dstip=`sudo nmap -sS -n -p ${box_port} -oG - ${localip}/24 2>/dev/null | awk '/open/{print $2}'`
        echo $dstip >$dstip_file
fi

echo ==============
cat $dstip_file
echo ==============

if [ "$dstip" != "" ]; then
        curl http://${dstip}:${box_port}/
fi

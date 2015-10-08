#!/bin/bash -
#===============================================================================
#          FILE: find.sh
#         USAGE: ./find.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/09/29 14:08
#===============================================================================

set -o nounset                              # Treat unset variables as an error

while read line; do
    echo $line
    ./multi_dns_query.py $line
    #./get_fastest_ip.sh ${line}"_result.log" >ip.log
    #sort -k2 -g ip.log -o ${line}"_ip_sort.log"
    ./multi_ping.py ${line}"_result.log" >${line}"_ip_sort.log"
    rm -f ${line}"_result.log"
done <$1


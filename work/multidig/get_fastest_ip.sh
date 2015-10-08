#!/bin/bash -
#===============================================================================
#          FILE: get_fastest_ip.sh
#         USAGE: ./get_fastest_ip.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/09/29 13:37
#===============================================================================

set -o nounset                              # Treat unset variables as an error

while read line; do
    arr=`ping -t 2 $line | grep icmp_seq | awk '{print $7}' | awk -F= '{print $2}'`
    echo -n $line
    echo $arr | awk '                  \
        BEGIN {sum = 0.0}              \
        {                              \
            for (i = 1;i <= NF; i++) { \
                sum += $i;             \
            }                          \
        }                              \
        END {if (NF > 0) printf(" %.2f\n", sum/NF); else printf(" 0\n");} \
    '
done < $1

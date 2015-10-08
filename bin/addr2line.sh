#!/bin/bash -
#===============================================================================
#          FILE: addr2line.sh
#         USAGE: ./addr2line.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014年08月26日 14:21
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. tools.sh

if [ $# -eq 0 ] || [ $# -gt 2 ]; then
    echo -n "Usage: `basename $0`"
    echo " bin_name addr"
    exit 0
fi

if [ $# -eq 2 ]; then
    # /usr/local/stratusee/lib/libaiehttp.so.1.0 0x8355
    bin_name=$1
    addr=$2
else
    # /usr/local/stratusee/lib/libaiehttp.so.1.0(+0x8355)
    bin_name=`echo $1 | awk -F"(" '{print $1}'`
    addr=`echo $1 | awk -F"+" '{print $2}' | awk -F")" '{print $1}'`
fi

echo addr2line -f -i -p -e $bin_name $addr | sed -e 's/:/ +/'
sudo addr2line -f -i -p -e $bin_name $addr | sed -e 's/:/ +/'


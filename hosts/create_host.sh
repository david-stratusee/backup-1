#!/bin/bash -
#===============================================================================
#          FILE: create_host.sh
#         USAGE: ./create_host.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年03月01日 08:54
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

while read line; do
    echo $line
    ipslist=`dig $line | grep -E "[	 ]A[	 ]" | awk '{print $5}'`
	for ip in ${ipslist}; do
		echo -e "$ip\t$line" >>hosts.all.bak
	done
done <$1

sort hosts.all.bak >hosts.all
rm -f hosts.all.bak

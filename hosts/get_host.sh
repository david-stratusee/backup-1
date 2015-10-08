#!/bin/bash -
#===============================================================================
#          FILE: get_host.sh
#         USAGE: ./get_host.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月28日 22:09
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

if [ $# -eq 0 ]; then
    hostname="hosts"
else
    hostname=$1
fi

dos2unix ${hostname}
grep -v "^#" ${hostname} | grep -v 127.0.0.1 | grep -v "::1" | grep -v "^$" | grep -v "255.255.255.255" | grep -v "aie\." | grep -v "stratusee" | awk '{print $2}' | uniq >domain.list

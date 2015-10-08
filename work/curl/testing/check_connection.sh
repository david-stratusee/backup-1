#!/bin/bash -
#===============================================================================
#          FILE: check_connext.sh
#         USAGE: ./check_connext.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月04日 08:59
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

port=$1
sudo netstat -anp | grep "192.168.10.102:${port}" | grep ESTABLISHED | awk '{print $4}' | awk -F":" '{print $2}' | sort

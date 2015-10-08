#!/bin/bash -
#===============================================================================
#          FILE: u.sh
#         USAGE: ./u.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/09/29 14:37
#===============================================================================

set -o nounset                              # Treat unset variables as an error

while read line; do
    echo ============================
    echo $line
    dig @${line} www.icloud.com
done <5

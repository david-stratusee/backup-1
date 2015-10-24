#!/bin/bash -
#===============================================================================
#          FILE: a.sh
#         USAGE: ./a.sh
#        AUTHOR: dengwei, david@holonetsecurity.com
#       CREATED: 2015年10月24日 06:24
#===============================================================================

set -o nounset                              # Treat unset variables as an error

for dir in $(/bin/ls); do
    echo $dir
    if [ -d $dir/.git ]; then
        cd $dir
        git pull
        cd ..
    fi
done

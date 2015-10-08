#!/bin/bash -
#===============================================================================
#          FILE: copy.sh
#         USAGE: ./copy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/01/23 15:30
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

list=`ls bash_*`
for file in $list; do
    echo colordiff $file ~/.$file
    colordiff $file ~/.$file
done

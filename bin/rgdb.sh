#!/bin/bash -
#===============================================================================
#          FILE: rgdb.sh
#         USAGE: ./rgdb.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 11/28/2014 17:47
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

process=`sudo file $1 | awk -F" from " '{print $2}' | awk '{print $1}' | awk -F"'" '{print $2}'`
echo gdb $process -c $1
sudo gdb $process -c $1


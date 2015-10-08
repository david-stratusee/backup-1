#!/bin/bash -
#===============================================================================
#          FILE: start.sh
#         USAGE: ./start.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月15日 05:28
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cur_dir=`pwd`

pkill.sh http.py
nohup ${cur_dir}/http.py "0.0.0.0:8088" 1>${cur_dir}/web.log 2>&1 &
ps -ef | grep -v grep | grep http.py

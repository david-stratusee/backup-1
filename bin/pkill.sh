#!/bin/bash -
#===============================================================================
#          FILE: pkill.sh
#         USAGE: ./pkill.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 07/11/2014 12:25
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ${HOME}/bin/tools.sh

kill_arg=""
if [ "$1" == "-9" ]; then
    kill_arg=" -9"
    shift
fi

LOG_FILE="/tmp/pkill.log"

pss | grep $1 | grep -v grep | grep -v pkill.sh > $LOG_FILE
echo ----------
cat $LOG_FILE
echo ----------

pid_num=`cat $LOG_FILE | wc -l`
if [ $pid_num -eq 0 ]; then
    echo no such process, exit ...
    exit 1
fi

pid=`cat $LOG_FILE | awk '{print $1}'`

echo "kill$kill_arg $pid"
sudo kill$kill_arg $pid

rm -f $LOG_FILE

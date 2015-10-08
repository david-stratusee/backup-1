#!/bin/bash -
#===============================================================================
#          FILE: git_push.sh
#         USAGE: ./git_push.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014年09月26日 10:49
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

result=1
while [ $result -gt 0 ]; do
    echo -n "[`date +'%m-%d %H:%M:%S'`] git pull"
    result_str=`git pull 2>&1`
    result=$?
    if [ $result -gt 0 ]; then
        color_print.py "red" " --- failed, $result_str"
        echo
        sleep 1
    else
        color_print.py "green" " --- ok"
        echo
    fi
done

result=1
while [ $result -gt 0 ]; do
    echo -n "[`date +'%m-%d %H:%M:%S'`] git push"
    result_str=`git push 2>&1`
    result=$?
    if [ $result -gt 0 ]; then
        color_print.py "red" " --- failed, $result_str"
        echo
        sleep 1
    else
        color_print.py "green" " --- ok"
        echo
    fi
done

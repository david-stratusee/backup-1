#!/bin/bash -
#===============================================================================
#          FILE: do_cscope.sh
#         USAGE: ./do_cscope.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 12/15/13 00:16
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#. ~/.bash_aliases

function do_help()
{
    echo "Help: $1 (level keyword)|-h"
    echo "-h: this help message"
    echo "level will be: "
    echo " 0: Find this C symbol"
    echo " 1: Find this global definition"
    echo " 2: Find functions called by this function"
    echo " 3: Find functions calling this function"
    echo " 4: Find this text string"
    echo " 5: Change this text string"
    echo " 6: Find this egrep pattern"
    echo " 7: Find this file"
    echo " 8: Find files #including this file"
    echo " 9: Find assignments to this symbol"
}

if [ $# -gt 0 ]; then
	if [ "$1" == "-h" ]; then
        do_help `basename $0`
		exit
	fi
else
    do_help `basename $0`
    exit
fi

num=$1
check_n=`echo $num | grep "^[0-9]$" | wc -l`
if [ $check_n -eq 0 ]; then
    echo "do_help `basename $0`"
    exit 1
fi
shift

echo cscope -L -d -$num \"$1\"
count=`cscope -L -d -$num "$1" | grep --color "$1" | wc -l`
if [ $count -eq 0 ]; then
    cscope -L -d -$num "$1"
else
    cscope -L -d -$num "$1" | grep --color "$1"
fi


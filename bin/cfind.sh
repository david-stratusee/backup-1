#!/bin/bash -
#===============================================================================
#          FILE:  vfind.sh
#   DESCRIPTION:
#
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#===============================================================================

. tools.sh

cache_file="./cscope.files"

if [ $# -lt 2 ]; then
	basefile=`basename $0`
	echo -e "Usage: \n\t$basefile filename action"
	echo -e "example: \n\t$basefile filename vi"
	exit 0
fi

count=0
if [ -f $cache_file ]; then
	filelist=`cat $cache_file | grep -E "$1"`
	count=`echo $filelist | awk '{print NF}'`
fi

if [ $count -eq 0 ]; then
	filelist=`find . -name $1`
	count=`echo $filelist | awk '{print NF}'`
fi

if [ $count -ne 1 ]; then
	echo "find $count file"
	if [ $count -ne 0 ]; then
		echo "-------------------"
		echo $filelist
		echo "-------------------"
	fi
	exit 1
fi

shift
action=`echo "$@" | sed -e "s/\bvi\b/vim/g"`

execute_hint $action $filelist


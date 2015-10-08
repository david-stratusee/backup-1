#!/bin/bash -
#===============================================================================
#          FILE:  check.sh
#   DESCRIPTION:  
# 
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:
#===============================================================================

#./dfp/dfp_http_decode.c +20 : #include "../common/string_decode.h" 2

src_file=$1
newdir=$2
newstr=$3

cur_dir=`pwd`
cat $src_file | while read line
do
	filename=`echo $line | awk '{print $1}'`
	headername=`echo $line | awk '{print $5}' | awk -F"\"" '{print $2}'`
	newheadername=`echo $headername | sed -e 's/vs_debug_func/vs_print_func/g'`
#	newheadername=`echo $headername | awk -F"/" '{print "debug/"$NF}' | sed -e "s/appd_/vs_/g"`
#	newheadername="$newdir/$newstr.h"

	headername=`echo $headername | sed -e 's/\//\\\\\//g'`
	newheadername=`echo $newheadername | sed -e 's/\//\\\\\//g'`

	echo "sed -i -e \"s/$headername/$newheadername/g\" $filename"
done


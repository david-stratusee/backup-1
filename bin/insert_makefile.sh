#!/bin/bash -

#if [ ! -f "Makefile.am" ]; then
#	echo "no find Makefile.am"
#	exit
#fi

curdir="."
prunedir=""
if [ $# -gt 0 ]; then
    if [ "$1" == "-d" ]; then
	    curdir=$2
    elif [ "$1" == "-p" ]; then
        prunedir="-path $curdir/$2 -prune -o"
    elif [ "$1" == "-h" ]; then
        echo "`basename $0` [-d dir | -p prune]"
        exit
    fi
fi

filelist=`find $curdir $prunedir -path $curdir/build -prune -o \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" \) -type f -printf "%P " | sort`
for file in $filelist
do
	echo "	\$(top_srcdir)/$file 	\\"
done


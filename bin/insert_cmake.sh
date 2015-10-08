#!/bin/bash -

#if [ ! -f "Makefile.am" ]; then
#	echo "no find Makefile.am"
#	exit
#fi

curdir="."
prunedir=""

while getopts 'd:p:h' opt; do
    case $opt in
        d) 
            curdir=$OPTARG;;
        p)
            prunedir="-path $curdir/$OPTARG -prune -o"
            ;;
        h|*)
            echo "`basename $0` [-d dir | -p prune]"
            exit 0;;
    esac
done

filelist=`find $curdir $prunedir -path $curdir/cbuild -prune -o \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" \) -type f -printf "%P\n" | sort -u`
for file in $filelist
do
	echo "	\${CMAKE_CURRENT_SOURCE_DIR}/$file"
done


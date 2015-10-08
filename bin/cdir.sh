#!/bin/bash -

#reindent the c files
#usage: put it in ~/bin, and using cdir.sh "filetype"

#filelist=`ls *.c *.h *.cc`
#filelist=`$1`

#for eachfile in $filelist
#do
#	indent -kr -i8 -ts8 -sob -l80 -ss -npsl -bad -l128 -lc128 $eachfile
#done

if [ $# -eq 0 ]
then
	echo "==================================="
	echo "Usage:	cdir.sh filetype"
	echo "cdir.sh \"*.[c|h]\""
	echo "cdir.sh \"*.c\""
	echo "cdir.sh c"
	echo "==================================="
	exit;
fi

if [ "$1" = "c" ] || [ "$1" = "C" ]
then
	echo "del temp files"
	find . -type f -name "[^\~]*\~" -exec rm -f {} \;
	exit
fi

find . -type f -name "$1" -exec cindent.sh {} \;
exit

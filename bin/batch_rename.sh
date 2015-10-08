#!/bin/bash -

if [ $# -lt 2 ]
then
	echo "Usage: `basename $0` src dst [dir]"
	exit
fi

if [ $# -eq 3 ]
then
	echo "directory is $3"
	dir=$3
else
	dir="."
fi

#list=`find $dir -name \"*$1*\" -type f`
#list=`ls $dir | grep -e \"$1\"`

ls $dir | grep -e "$1" | while read file
do
	newfile=`echo $file | sed s/$1/$2/g`
#	echo $file $newfile
	mv "$dir/$file" "$dir/$newfile" -f 
done


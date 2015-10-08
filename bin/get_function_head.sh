#!/bin/bash -

if [ $# -eq 0 ]
then
	echo "`basename $0` filename"
	exit
fi

egrep "^([0-9A-Za-z_]+[ \t]+)+[0-9A-Za-z_*]+\([0-9A-Za-z_ \t\*,]*\)$" $@ | grep -v extern | awk '{print $0";"}' | grep -v static

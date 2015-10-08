#!/bin/bash -

if [ $# -eq 0 ]; then
	dir="."
else
	dir=$1
fi

find $dir -type f -exec dos2unix -qk {} \;

#!/bin/bash -

#reindent the c file
#usage: put it in ~/bin, and using cindent.sh filename
#the script can only parse one.
#if you want to parse more, please use cdir.sh

if [ $# -eq 0 ]
then
	echo "==================================="
	echo "Usage: cindent filename"
	echo "this script can only parse one file"
	echo "==================================="
	exit;
fi

dos2unix $1
indent -kr -i4 -ts4 -sob -ss -npsl -bad -l128 -lc128 -nut -nv $1
#vim $1 -s ~/bin/myscript
# BREAKING LONG LINES
#       -bbo, --break-before-boolean-operator
#                  Prefer to break long lines before boolean operators.
#       -nhnl, --ignore-newlines
#                  Do not prefer to break long lines at the position of newlines in the input.
#       -ln, --line-lengthn
#                  Set maximum line length for non-comment lines to n.
#
#       --preprocessor-indentationn               -ppin

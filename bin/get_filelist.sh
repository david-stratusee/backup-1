#!/bin/bash -
#===============================================================================
#          FILE:  get_filelist.sh
#   DESCRIPTION:  get file list for c files
#
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:
#===============================================================================

trap 'cd $fl_cur_dir; rm -rf $FILE_LIST_DIR; exit;' INT TERM

FILE_LIST_DIR="./file_list_dir"

fl_cur_dir=`pwd`
get_file_list()
{
	fl_cur_dir=`pwd`
	if [ ! -d $FILE_LIST_DIR ]; then
		mkdir $FILE_LIST_DIR
	fi

	file_list=$FILE_LIST_DIR/$1
	echo -n "" > $file_list

    find . \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.h" \) -type f >> $file_list

	echo "collect file list in $file_list"
}


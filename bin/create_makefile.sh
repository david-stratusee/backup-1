#!/bin/bash -
#===============================================================================
#          FILE:  create_makefile.sh
#   DESCRIPTION:  
# 
#        AUTHOR:  dengwei 
#       VERSION:  1.0
#===============================================================================

if [ $# -lt 1 ]; then
	echo -e "Usage:\n\t`basename $0` exec_name"
	exit
fi

exec_name=$1
filename=Makefile

echo -n "" > $filename

echo -n "CC=gcc
CFLAGS=-Wall -Werror -O2
all: $exec_name

$exec_name: " >> $filename

echo `ls *.c` >> $filename
echo -e "\t\$(CC) \$(CFLAGS) -o \$@ \$^" >> $filename
echo -e "\nclean:
	rm -f $exec_name *.o" >> $filename

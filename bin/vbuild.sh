#!/bin/bash -

# script for making the file
# mybuild.sh [-c|-h|-q]
# ${!#} means the last parameters

. tools.sh

if [ ! -f "Makefile.am" ]; then
	echo "Not find Makefile.am, exit!"
	exit 1;
fi

BIN_NUM=`cat Makefile.am | grep _PROGRAMS | wc -l`
if [ $BIN_NUM -ne 0 ]; then
#	BIN_NAME=`cat configure.in | grep AM_INIT_AUTOMAKE | awk -F"(" '{print $2}' | awk -F"," '{print $1}'`
	BIN_NAME=`cat Makefile.am | grep _PROGRAMS | awk -F"=" '{print $2}'`
else
#	BIN_NAME=`cat Makefile.am | grep _LIBRARIES | awk -F"=" '{print $2}' | awk -F" " '{print $1}' | awk -F"	" '{print $1}'`
	BIN_NAME=`cat Makefile.am | grep _LIBRARIES | awk -F"=" '{print $2}'`
fi

REBUILD_SH="rebuild.sh"

if [ -f "./$REBUILD_SH" ]; then
	echo "execute ./$REBUILD_SH"
	chmod +x ./$REBUILD_SH
	REBUILD_SH="./$REBUILD_SH"
else
	echo "execute $REBUILD_SH"
	REBUILD_SH="$REBUILD_SH"
fi

if [ "$1" = "-h" ] || [ "$1" = "-H" ]; then
	execute "$REBUILD_SH $1"
	exit 0;
fi

rm -f $BIN_NAME
if [ "$1" = "-c" ] || [ "$1" = "-C" ]; then
	execute "$REBUILD_SH $1"
	exit 0;
fi

if [ "$1" = "-r" ] || [ "$1" = "-R" ]; then
	execute "$0 -c"
	execute "$0 -q"
	exit 0;
fi

MAKE_DIR=build
if [ ! -d $MAKE_DIR ]; then
	echo "Not find dir $MAKE_DIR, execute $REBUILD_SH"
	execute "$REBUILD_SH $1"
	ret=$?
	if [ $ret -ne 0 ]; then
		echo "error when $REBUILD_SH $1"
		exit $ret
	fi
fi

if [ ! -d $MAKE_DIR ]; then
	echo "still not find dir $MAKE_DIR, exit 1"
	exit 1;
fi

cd $MAKE_DIR
if [ "$1" = "-q" ] || [ "$1" = "-Q" ]; then
	execute "make -s $BIN_NAME"
else
	execute "make $BIN_NAME"
fi

ret=$?
if [ $ret -ne 0 ]; then
	echo "ERROR when make, in `pwd`"
	cd ..
	exit $ret
else
	echo "make ok, copy $BIN_NAME"
	cp $BIN_NAME ..
	cd ..
	exit 0
fi

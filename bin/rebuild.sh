#!/bin/bash -

cecho ()               # Color-echo.
		       # Argument $1 = message
		       # Argument $2 = color
{
	local default_msg="No message passed."
			       # Doesn't really need to be a local variable.

	message=${1:-$default_msg}   # Defaults to default message.

	echo -ne "$message"
	tput sgr0                      # Reset to normal.

	return
}

cechodone ()
{
	echo -n " [ "
	cecho "done"
	echo " ]"
}

BUILD_DIR=build
ERR_LOG_FILE="`pwd`/rebuild_err.log"
>$ERR_LOG_FILE	# empty the logfile

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
	cecho "`basename $0`: " $default_color
	echo
	cecho "-h: help information;" $default_color
	echo
	cecho "-q: quiet status;" $default_color
	echo
	cecho "-c: remove temporary files;" $default_color
	echo
	exit 0
fi

if [ "$1" = "-c" ] || [ "$1" = "-C" ]; then
	cecho "remove temporary files ..." $default_color
	rm -rf config.guess config.sub depcomp compile Makefile.in install-sh missing mkinstalldirs $BUILD_DIR autom4te.cache  configure aclocal.m4 .deps $ERR_LOG_FILE
	cechodone
	exit 0
fi

if [ ! -f "Makefile.am" ]; then
	echo "ERROR: can not find Makefile.am, exit"
	exit 1
fi

if [ ! -f "configure.in" ] && [ ! -f "configure.ac" ]; then
	echo "ERROR: can not find configure.in or configure.ac, exit"
	exit 1
fi

quiet_status=""
if [ "$1" = "-q" ] || [ "$1" = "-Q" ]; then
	quiet_status="-q"
fi

#########################
# begin to compile
#########################

cecho "aclocal ..." $default_color
aclocal 2>>$ERR_LOG_FILE
ret=$?
if [ $ret -ne 0 ]; then
	echo "ERROR when aclocal, see $ERR_LOG_FILE"
	exit $ret
fi
cechodone

cecho "autoconf ..." $default_color
autoconf 2>>$ERR_LOG_FILE
ret=$?
if [ $ret -ne 0 ]; then
	echo "ERROR when autoconf, see $ERR_LOG_FILE"
	exit $ret
fi
cechodone

cecho "automake --add-missing ..." $default_color
automake --add-missing 2>>$ERR_LOG_FILE
ret=$?
if [ $ret -ne 0 ]; then
	echo
	echo "ERROR when \"automake --add-missing\", see $ERR_LOG_FILE"
	exit $ret
fi
cechodone

if [ -d $BUILD_DIR ]; then
	cecho "rm -rf $BUILD_DIR" $default_color
	rm -rf $BUILD_DIR
	cechodone
fi

cecho "mkdir $BUILD_DIR" $default_color
mkdir $BUILD_DIR
cechodone

cd $BUILD_DIR

if [ ! -x "../configure" ]; then
	chmod +x ../configure
	cechodone
fi

cecho "../configure $quiet_status" $default_color
cechodone
../configure $quiet_status 2>>$ERR_LOG_FILE
ret=$?
if [ $ret -ne 0 ]; then
	echo "ERROR when configure, see $ERR_LOG_FILE"
	exit $ret
fi

if [ ! -f "../Makefile.in" ]; then
	echo "ERROR: no Makefile.in at ../"
	exit 1
fi

cd ..
echo 
echo "****** please go to directory [$BUILD_DIR], and execute make ******"
echo

#success, delete the $ERR_LOG_FILE
rm -f $ERR_LOG_FILE

exit 0


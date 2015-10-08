#!/bin/bash -

#  %u   day of week (1..7); 1 is Monday

week_date=`date +%u`
if [ $week_date -eq 1 ]; then
	echo -n "Mon "
elif [ $week_date -eq 2 ]; then
	echo -n "Tue "
elif [ $week_date -eq 3 ]; then
	echo -n "Wed "
elif [ $week_date -eq 4 ]; then
	echo -n "Thu "
elif [ $week_date -eq 5 ]; then
	echo -n "Fri "
elif [ $week_date -eq 6 ]; then
	echo -n "Sat "
elif [ $week_date -eq 7 ]; then
	echo -n "Sun "
fi

myname=`basename $HOME`

echo "`date +"%Y-%m-%d %H:%M"` crazyman"
echo "[Cause]"
echo "	*"
echo
echo "[Treatment]"
echo
echo

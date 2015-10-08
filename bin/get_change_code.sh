#!/bin/bash -

#cvs_check.sh | awk '{print $2}' | egrep "*\.[c|h]$"

cvs up | egrep ".*(\.c|\.h|\.am)$" | while read line
do
	first=`echo $line | awk '{print $1}'`
	second=`echo $line | awk '{print $2}'`

	if [ "$first" = "?" ] || [ "$first" = "1" ]
	then
		echo "	*	"$second"(A):"
	elif [ "$first" = "M" ]
	then
		echo "	*	"$second"(M):"
	fi

done


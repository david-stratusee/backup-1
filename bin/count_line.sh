#!/bin/bash -

#total_line=`find . -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" -type f -exec cat {} \; | grep -v "^[ |	]*//.*" | wc -l`
total_line=`find . \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" \) -type f -exec cat {} \; | wc -l`
# | awk '{print $1}' | while read line
#do
#	total_line=`expr $total_line + $line`
#	echo $total_line
#done

#echo "============="
echo $total_line

########total_line=0
########exp_begin=0
########exp_end=0
########exp_cont=0

########find . -type f -name "*.[c|h]" -exec cat {} \; | while read line
########do
########	exp_cont=`echo "$line" | grep "^[ |      ]*//" | wc -l`
########	if [ $exp_cont -eq 1 ]; then
########		continue;
########	fi

########	if [ $exp_begin -eq 0 ]; then
########		exp_begin=`echo "$line" | grep "^[ |	]*/\*" | wc -l`
########	fi

########	if [ $exp_begin -eq 1 ]; then
########		exp_end=`echo "$line" | grep "\*/" | wc -l`
########		if [ $exp_end -eq 1 ]; then
########			exp_begin=0
########		fi
########	else
########		total_line=`expr $total_line + 1`
########		echo $exp_begin" "$total_line" ""$line"
########	fi
########done

########echo $total_line


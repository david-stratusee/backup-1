#!/bin/bash -

if [ $# -lt 2 ]; then
	echo "`basename $0` diff_file next_dir"
	exit;
fi

next_dir=$2
diff_file=$1

echo "diff $diff_file $next_dir/$diff_file"

diff.sh $diff_file $next_dir/$diff_file


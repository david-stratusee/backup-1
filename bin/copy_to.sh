#!/bin/bash -

if [ $# -lt 2 ]; then
	echo "$0 filename dest_dir";
	exit;
fi

filename=$1
dest_dir=$2

action="cp $filename $dest_dir/$filename"
echo $action
$action

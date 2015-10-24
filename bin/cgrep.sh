#!/bin/bash -

if [ $# -eq 0 ]
then
	echo "`basename $0` search_string"
	exit
fi

echo "------------------------------"

#grep -r -H -E -n "$@" . --exclude-dir=tmp --exclude-dir=temp --include="*.c" --include="*.h" --include="*.cc" --include="*.cpp" --include="*.hpp" | sed -e 's/:/ +/' | grep -E --color "$@"
grep -r -H -E -n "$@" . --exclude-dir=tmp --exclude-dir=temp --include="*.c" --include="*.h" --include="*.cc" --include="*.cpp" --include="*.hpp" | sed -e 's/:/ +/'

echo "------------------------------"

exit 0


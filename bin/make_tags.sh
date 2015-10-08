#!/bin/bash -

if [ $# -eq 0 ]
then
    ctags -R `find . \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" \) -type f`
else
    ctags -R `find . \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" \) -type f | grep -v $1`
fi

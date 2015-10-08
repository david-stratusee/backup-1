#!/bin/bash -
#===============================================================================
#          FILE: make.sh
#         USAGE: ./make.sh
#        AUTHOR: dengwei, david@holonetsecurity.com
#       CREATED: 2015年09月16日 00:31
#===============================================================================

set -o nounset                              # Treat unset variables as an error

gcc test.c md5.c md5.h
mv a.out ~/bin/mymd5 
rm -f *.gch

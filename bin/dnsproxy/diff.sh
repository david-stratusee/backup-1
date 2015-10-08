#!/bin/bash -
#===============================================================================
#          FILE: copy.sh
#         USAGE: ./copy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年08月25日 17:23
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

files=`ls dnschef.*`
for file in $files; do
    echo diff $file ~/bin/$file
    diff $file ~/bin/$file
done

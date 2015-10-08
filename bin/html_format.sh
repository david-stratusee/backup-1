#!/bin/bash -
#===============================================================================
#          FILE: html_format.sh
#         USAGE: ./html_format.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED:
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
. tools.sh

if [ $# -lt 2 ]; then
	echo "`basename $0` input output"
	exit
fi

execute xmllint --htmlout --html --nowarning --nowrap --timing --output $2 $1


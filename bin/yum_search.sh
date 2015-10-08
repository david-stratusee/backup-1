#!/bin/bash -
#===============================================================================
#          FILE: yum_search.sh
#         USAGE: ./yum_search.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年08月15日 00:28
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

yum search $1
echo ========================
yum provides */$1

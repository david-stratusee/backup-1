#!/bin/bash -
#===============================================================================
#          FILE: make.sh
#         USAGE: ./make.sh
#        AUTHOR: dengwei, david@holonetsecurity.com
#       CREATED: 2015年10月19日 17:37
#===============================================================================

set -o nounset                              # Treat unset variables as an error

gcc -O2 -o sp_bin sp_bin.c

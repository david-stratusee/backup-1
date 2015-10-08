#!/bin/bash -
#===============================================================================
#          FILE: default-difftool.sh
#         USAGE: ./default-difftool.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 08/12/2014 10:49
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#colordiff -E -b -w -B -u -p $@
#colordiff -y -E -b -w -W 200 $@ | less -RF

vimdiff $@

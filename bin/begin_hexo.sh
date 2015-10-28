#!/bin/bash -
#===============================================================================
#          FILE: start_hexo.sh
#         USAGE: ./start_hexo.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ${HOME}/bin/tools.sh

cd $HOME/Documents/blog
hexo server 1>>$HOME/log/hexo_error.log 2>&1 &

pss | grep "hexo server" | grep -v grep


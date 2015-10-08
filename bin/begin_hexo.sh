#!/bin/bash -
#===============================================================================
#          FILE: start_hexo.sh
#         USAGE: ./start_hexo.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cd $HOME/Documents/blog
hexo server 1>>$HOME/log/hexo_error.log 2>&1 &

ps axuf | grep "hexo server" | grep -v grep


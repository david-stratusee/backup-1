#!/bin/bash -
#===============================================================================
#          FILE: create_sys_ctags.sh
#         USAGE: ./create_sys_ctags.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED:
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

ctags -I __THROW --file-scope=yes --langmap=c:+.h --languages=c --c-kinds=+p -R -f ~/.vim/systags /usr/include /usr/local/include


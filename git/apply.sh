#!/bin/bash -
#===============================================================================
#          FILE: apply.sh
#         USAGE: ./apply.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年07月19日 00:45
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cp netrc ~/.netrc
cp gitconfig ~/.gitconfig
mkdir -p ~/.config/git/
cp ignore ~/.config/git/ignore

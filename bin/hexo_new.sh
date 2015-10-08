#!/bin/bash -
#===============================================================================
#          FILE: hexo_new.sh
#         USAGE: ./hexo_new.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

newlog=`hexo new $1`

#newlog="[info] File created at /home/dengwei/Documents/blog/source/_posts/2014_02_08_stackedit.md"
filename=`echo $newlog | awk '{print $NF}'`

vim $filename

echo $filename

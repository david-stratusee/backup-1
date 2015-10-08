#!/bin/bash -
#===============================================================================
#          FILE:  remove_emptyline.sh
#   DESCRIPTION:  remove empty line
#
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:
#===============================================================================

sed /^$/d $1 > /tmp/.remove_empty_line
mv -f /tmp/.remove_empty_line $1


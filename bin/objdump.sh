#!/bin/bash -
#===============================================================================
#          FILE:  objdump.sh
#   DESCRIPTION:
#
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:
#===============================================================================

objdump -D -S -s -x -t -T $1

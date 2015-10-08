#!/bin/bash -
#===============================================================================
#          FILE:  make_kernel.sh
#   DESCRIPTION:  
# 
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:
#===============================================================================

make && make modules && make modules_install && make install

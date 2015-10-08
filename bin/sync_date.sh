#!/bin/bash -
#===============================================================================
#          FILE: ntp.sh
#         USAGE: ./ntp.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 12/15/2014 17:14
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sudo ntpdate ntp.ubuntu.com

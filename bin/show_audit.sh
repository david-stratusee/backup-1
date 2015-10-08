#!/bin/bash -
#===============================================================================
#          FILE: show_audit.sh
#         USAGE: ./show_audit.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年08月19日 11:13
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sudo cat /var/log/audit/audit.log | ausearch -i


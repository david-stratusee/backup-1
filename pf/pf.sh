#!/bin/bash -
#===============================================================================
#          FILE: pf.sh
#         USAGE: ./pf.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/01/16 13:46
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

rdr pass on lo0 proto tcp from any to any port 80 -> 127.0.0.1 port 3128
rdr pass on lo0 proto tcp from any to any port 443 -> 127.0.0.1 port 3129

pass out on en0 route-to lo0 inet proto tcp from any to any port 80 keep state
pass out on en0 route-to lo0 inet proto tcp from any to any port 443 keep state

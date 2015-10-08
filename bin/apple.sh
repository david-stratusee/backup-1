#!/bin/bash -
#===============================================================================
#          FILE: dns.sh
#         USAGE: ./dns.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年07月06日 13:45
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sudo systemctl $1 dnsmasq.service
ps -ef | grep -v grep | grep dnsmasq

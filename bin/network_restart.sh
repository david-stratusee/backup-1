#!/bin/bash -
#===============================================================================
#          FILE: network_restart.sh
#         USAGE: ./network_restart.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 11/26/2014 20:55
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ${HOME}/bin/tools.sh

ifname=eno16777736

sudo ifdown ${ifname} && sudo ifup ${ifname}
sudo kill $(pss | grep "sslsplit " | grep -v grep | awk '{print $2}')

ifconfig

#!/bin/bash -
#===============================================================================
#          FILE: icmp_redirect.sh
#         USAGE: ./icmp_redirect.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/10/30 15:11
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sysctl -w net.ipv4.conf.all.accept_redirects="0"
sysctl -w net.ipv4.conf.all.send_redirects="0"
sysctl -w net.ipv4.conf.default.accept_redirects="0"
sysctl -w net.ipv4.conf.default.send_redirects="0"
sysctl -w net.ipv4.conf.eth0.accept_redirects="0"
sysctl -w net.ipv4.conf.eth0.send_redirects="0"

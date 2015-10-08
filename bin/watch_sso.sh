#!/bin/bash -
#===============================================================================
#          FILE: watch_sso.sh
#         USAGE: ./watch_sso.sh
#        AUTHOR: dengwei, david@stratusee.com
#       CREATED: 2015/09/17 23:03
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cd ${HOME}/work/openshift/shadowsocks
while :
do
    pidc=`ps -ef | grep -v grep | grep -c "local.js"`
    if [ $pidc -eq 0 ]; then
        nohup node local.js -s "wss://shadowsocks-crazyman.rhcloud.com:8443" 1>/dev/null 2>/tmp/shadowsocks-error.log &
    fi

    sleep 1
done

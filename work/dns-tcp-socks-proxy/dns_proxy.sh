#!/bin/bash -
#===============================================================================
#          FILE: dns_proxy.sh
#         USAGE: ./dns_proxy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/01/11 15:48
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

while getopts 'hcl' opt; do
    case $opt in
        c)
            sudo networksetup -setdnsservers Wi-Fi empty
            sudo dscacheutil -flushcache
            sudo pkill dns_proxy
            exit 0
            ;;
        l)
            ps -ef | grep -v grep | grep -v dns_proxy.sh | grep --color dns_proxy
            exit 0
            ;;
        h|*)
            echo "$0 [-c|-l]"
            exit 0
            ;;
    esac
done
$0 -c

#ulimit -c unlimited

if [ ! -f /tmp/proxy.list ]; then
    wget https://david-stratusee.github.io/proxy.pac -P /tmp/
    if [ $? -ne 0 ]; then
        echo "download file failed, check the network"
        exit 1
    fi

    grep "\":[ ]1[,]$" /tmp/proxy.pac | awk -F"\"" '{print $2}' >/tmp/proxy.list
    extra_list="wikipedia.org ggpht.com google"
    for node in ${extra_list}; do
        echo $node >> /tmp/proxy.list
    done
    rm -f proxy.pac
fi

cp -f /etc/resolv.conf /tmp/resolv.conf

homedir=${HOME}
sudo ${homedir}/bin/dns_proxy ${homedir}/.dns_proxy.conf

# set dns to local
sudo networksetup -setdnsservers Wi-Fi 127.0.0.1
sudo dscacheutil -flushcache


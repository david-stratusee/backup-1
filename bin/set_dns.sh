#!/bin/bash -
#===============================================================================
#          FILE: set_dns.sh
#         USAGE: ./set_dns.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014/12/14 11:37
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

which lsb_release >/dev/null
is_mac=$?

if [ $# -gt 0 ] && [ "$1" == "-h" ]; then
    echo "default is set dns, -c means clear dns, -l means list dns."
    echo "for opendns: 208.67.222.222 208.67.220.220"
    echo "for GOOGLE: 8.8.8.8 8.8.4.4"
    if [ ${is_mac} -ne 0 ]; then
        echo "All for Wi-Fi"
    fi
    exit 0
fi

if [ $# -gt 0 ] && [ "$1" == "-c" ]; then
    if [ ${is_mac} -ne 0 ]; then
        sudo networksetup -setdnsservers Wi-Fi empty
        sudo dscacheutil -flushcache
    elif [ -d /etc/resolvconf/ ]; then
        sudo sed -i '/^nameserver .*$/d' /etc/resolvconf/resolv.conf.d/head
        sudo resolvconf -u
    else
        if [ -f /etc/resolv.conf.bak ]; then
            sudo mv -f /etc/resolv.conf.bak /etc/resolv.conf
        else
            echo "no dns resolv files for recovery"
        fi
    fi

    $0 -l
    exit 0
fi

if [ $# -gt 0 ] && [ "$1" == "-l" ]; then
    echo -n "Show DNS: "
    if [ ${is_mac} -ne 0 ]; then
        networksetup -getdnsservers Wi-Fi
    else
        grep -v "^# " /etc/resolv.conf
    fi
    exit 0
fi

setdns="192.168.0.2"
if [ $# -gt 0 ]; then
    setdns=$@
fi

if [ ${is_mac} -ne 0 ]; then
    sudo networksetup -setdnsservers Wi-Fi ${setdns}
    sudo dscacheutil -flushcache
elif [ -d /etc/resolvconf/ ]; then
    for node in $setdns; do
        sudo bash -c "echo nameserver $node >> /etc/resolvconf/resolv.conf.d/head"
    done
    sudo resolvconf -u
else
    if [ ! -f /etc/resolv.conf.bak ]; then
        sudo cp -f /etc/resolv.conf /etc/resolv.conf.bak
    fi

    sudo sed -i '/^nameserver .*$/d' /etc/resolv.conf
    for node in $setdns; do
        sudo bash -c "echo nameserver $node >> /etc/resolv.conf"
    done
fi

$0 -l


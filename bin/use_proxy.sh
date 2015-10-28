#!/bin/bash -
#===============================================================================
#          FILE: use_proxy.sh
#         USAGE: ./use_proxy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/04/24 14:08
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ${HOME}/bin/tools.sh

netname="en0"
os_name=`uname -s`
is_linux=0

NORMAL_MODE=0
CLEAR_MODE=1
SHOW_MODE=2

BOX=0
VM=1

function proxy_help()
{
    echo "$1 -c: delete proxy and restore route"
    echo "$1 -l: show proxy route"
    if [ ${is_linux} -eq 0 ]; then
        echo "$1 -v: use vmware local proxy"
    fi
    echo "$1 -b: use box aie proxy"
    echo "$1 -d: use dns proxy"
}

function reset_gw()
{
    if [ ${is_linux} -gt 0 ]; then
        echo route delete -net 0.0.0.0/0
        sudo route delete -net 0.0.0.0/0

        echo route add -net 0.0.0.0/0 gw $1
        sudo route add -net 0.0.0.0/0 gw $1
    else
        echo route -n delete -net 0.0.0.0/0
        sudo route -n delete -net 0.0.0.0/0

        echo route -n add -net 0.0.0.0/0 $1
        sudo route -n add -net 0.0.0.0/0 $1
    fi
}

if [ "${os_name}" == "Linux" ] || [ "${os_name}" == "linux" ]; then
    is_linux=1
    netname="eno16777736"
fi

if [ $# -eq 0 ] || [ "$1" == "-h" ]; then
    proxy_help `basename $0`
    exit 0
fi

mode=${SHOW_MODE}
dest=${BOX}
dns_set=0
while getopts 'cldbv' opt; do
    case $opt in
        c)
            mode=${CLEAR_MODE}
            ;;
        l)
            mode=${SHOW_MODE}
            ;;
        b)
            mode=${NORMAL_MODE}
            dst=${BOX}
            ;;
        v)
            if [ ${is_linux} -ne 0 ]; then
                proxy_help
                exit 0
            fi

            mode=${NORMAL_MODE}
            dst=${VM}
            ;;
        d)
            dns_set=1
            ;;
        *)
            proxy_help
            exit 1
            ;;
    esac
done

if [ $mode -eq ${CLEAR_MODE} ]; then
    local_ip=`ifconfig ${netname} | egrep -o "inet[ \t]+.*[ \t]+netmask" | awk '{print $2}'`
    gw=`echo $local_ip | awk -F"." '{print $1"."$2"."$3".1"}'`
    echo ${gw}
    reset_gw ${gw}
    #if [ $dns_set -ne 0 ]; then
        set_dns.sh -c
    #fi

    if [ -f /etc/sysconfig/network-scripts/ifcfg-${netname} ]; then
        sudo sed -i -e 's/^DEFROUTE=.*/DEFROUTE="yes"/g' /etc/sysconfig/network-scripts/ifcfg-${netname}
        dhpid=`pss | grep dhcl | grep ${netname} | awk '{print $2}'`
        if [ "$dhpid" != "" ]; then
            sudo kill $dhpid
        fi
        sudo dhclient ${netname}
    fi
elif [ $mode -eq ${NORMAL_MODE} ]; then
    if [ ${dst} == ${VM} ]; then
        reset_gw 192.168.66.128
        if [ $dns_set -ne 0 ]; then
            set_dns.sh 192.168.66.128
        fi
    elif [ ${dst} == ${BOX} ]; then
        if [ ${is_linux} -ne 0 ] && [ -f /etc/sysconfig/network-scripts/ifcfg-${netname} ]; then
            sudo sed -i -e 's/^DEFROUTE=.*/DEFROUTE="no"/g' /etc/sysconfig/network-scripts/ifcfg-${netname}
            dhpid=`pss | grep dhcl | grep ${netname} | awk '{print $2}'`
            if [ "$dhpid" != "" ]; then
                sudo kill $dhpid
            fi
            sudo dhclient ${netname}
        fi

        box_ip=`grep aie.box /etc/hosts | awk '{print $1}'`
        reset_gw ${box_ip}
        if [ $dns_set -ne 0 ]; then
            set_dns.sh ${box_ip}
        fi
    fi
elif [ $mode -ne ${SHOW_MODE} ]; then
    echo "unknown argument"
    proxy_help `basename $0`
    exit 1
fi

netstat -nr
set_dns.sh -l

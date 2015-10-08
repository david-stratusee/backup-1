#!/bin/bash -
#===============================================================================
#          FILE: new_socks.sh
#         USAGE: ./new_socks.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/01/05 21:18
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
. tools.sh

#-------------------------------------------------------------------------------
# config here
# 1. modify username
# 2. add proxy.pac to /Library/WebServer/Documents/proxy.pac
#-------------------------------------------------------------------------------
username=david

available_host_port=([0]="david:us.stratusee.com:2226" [1]="55e6658f0c1e66d617000070:shadowsocks-crazyman.rhcloud.com:22")
host_port=${available_host_port[1]}

ETH="Wi-Fi"
aliveinterval=0
USE_SSH=0
#-------------------------------------------------------------------------------
# config end
#-------------------------------------------------------------------------------

function show_proxy()
{
    echo ===========================
    echo proxy.pac: ${local_proxydir}
    echo "PAC_PROXY STATE:"
    networksetup -getautoproxyurl ${ETH}
    echo ===========================
    echo "PROCESS INFO:[${USE_SSH}]"
    ps -ef | grep -v grep | egrep --color=auto "(ssh -D|CMD|local.js|httpd|watch_socks|watch_sso)"
    echo ===========================
    if [ ${USE_SSH} -ne 0 ] && [ -f /tmp/watch_socks.log ]; then
        echo "/tmp/watch_socks.log:"
        grep "ssh -D" /tmp/watch_socks.log
        echo ===========================
    elif [ ${USE_SSH} -eq 0 ]; then
        echo "LISTEN INFO:"
        netstat -anb | grep 15500 | egrep --color=auto -i "(listen|established)"
        echo ===========================
        if [ -f /tmp/shadowsocks.log ]; then
            echo "/tmp/shadowsocks.log:"
            cat /tmp/shadowsocks.log
        fi
        echo ===========================
    fi
}

function fill_and_run_proxy()
{
    if [ ${USE_SSH} -eq 0 ]; then
        watch_sso.sh &
    else
        username=`echo ${host_port} | awk -F":" '{print $1}'`
        remote_host=`echo ${host_port} | awk -F":" '{print $2}'`
        remote_port=`echo ${host_port} | awk -F":" '{print $3}'`
        remote_ip=`get_dnsip ${remote_host}`

        echo "get host: $remote_host - $remote_ip" >/tmp/watch_socks.log
        watch_socks.sh ${username} ${remote_ip} ${remote_port} ${aliveinterval} >>/tmp/watch_socks.log 2>&1 &
    fi
}

function kill_process()
{
    pidc=`ps -ef | grep -v grep | grep -c "$@"`
    if [ $pidc -gt 0 ]; then
        sshpid=`ps -ef | grep "$@" | grep -v grep | awk '{print $2}'`
        for p in $sshpid; do
            echo "kill $@, pid: ${p}"
            sudo kill $p
        done
    fi
}

function print_avail_host()
{
    nhost=0
    echo "available socks host-port is:[${#available_host_port[*]}]"
    for node in ${available_host_port[*]}; do
        if [ "${host_port}" == "${node}" ]; then
            echo " [${nhost}] $node [***]"
        else
            echo " [${nhost}] $node"
        fi

        nhost=`expr ${nhost} + 1`
    done
}

function check_host_port()
{
    check_value=$1
    for node in ${available_host_port[*]}; do
        if [ "${check_value}" == "${node}" ]; then
            return 0
        fi
    done

    return 1
}

function update_pac()
{
    has_curl=$1
    if [ $has_curl -ne 0 ]; then
        rm -f /tmp/proxy.pac
        echo -n curl --connect-timeout 30 -s http://david-stratusee.github.io/proxy.pac -o /tmp/proxy.pac
        curl --connect-timeout 30 -s http://david-stratusee.github.io/proxy.pac -o /tmp/proxy.pac

        echo " -- [$?]"

        #echo wget -T 10 -nv http://david-stratusee.github.io/proxy.pac -P /tmp/
        #wget -T 10 -nv http://david-stratusee.github.io/proxy.pac -P /tmp/
        if [ $? -eq 0 ]; then
            sudo mv /tmp/proxy.pac ${local_proxydir}/
        elif [ ! -f ${local_proxydir}/proxy.pac ]; then
            echo "can not get proxy.pac, exit..."
            return 1
        else
            echo "can not get proxy.pac from github, so just use old one"
        fi
    fi

    return 0
}

function stop_apache()
{
    httpd_count=`ps -ef | grep -v grep | grep -c httpd`
    if [ ${httpd_count} -gt 0 ]; then
        sudo apachectl graceful-stop
    fi
}

function start_apache()
{
    stop_apache
    sudo apachectl start
}

function aws_socks_help()
{
    echo "------------------------------------"
    echo "Help Usage     : "

    echo -e "\n### for proxy ###"
    echo "-l             : for query socks proxy"
    echo "-c             : for clear socks proxy"
    echo "-r             : for reboot socks proxy"

    echo -e "\n### for PAC   ###"
    echo "-e ETH         : for ETH-TYPE, default Wi-Fi, only used by MacOS"
    echo "-f             : for local file for pac, only for Safari"

    echo -e "\n### for SSH   ###"
    echo "-s             : ssh mode"
    echo "-a NUM         : set ServerAliveInterval for sshtunnel, default 0, recommand 7200"
    echo "-p NUM|IP:PORT : set socks proxy's host_port"
    print_avail_host

    echo "no args for set socks proxy and DIRECT"
    echo "------------------------------------"
}

MODE="normal"
restart=0
use_local_web=1
while getopts 'a:e:p:hcrlfs' opt; do
    case $opt in
        # for proxy
        c|r)
            if [ "${MODE}" == "normal" ]; then
                MODE="clear"
                if [ "$opt" == 'r' ]; then
                    restart=1
                fi
            else
                echo "clear and query mode should not be used at same time"
                aws_socks_help
                exit 1
            fi
            ;;
        l)
            if [ "${MODE}" == "normal" ]; then
                MODE="query"
            else
                echo "clear and query mode should not be used at same time"
                aws_socks_help
                exit 1
            fi
            ;;

        # for PAC
        e)
            ETH=$OPTARG
            ;;
        f)
            use_local_web=0
            ;;

        # for ssh
        s)
            USE_SSH=1
            ;;
        a)
            aliveinterval=$OPTARG
            ;;
        p)
            isdigit=`echo $OPTARG | grep -c "^[0-9]$"`
            if [ $isdigit -gt 0 ] && [ $OPTARG -lt ${#available_host_port[*]} ]; then
                host_port=${available_host_port[$OPTARG]}
            else
                check_host_port $OPTARG
                if [ $? -eq 0 ]; then
                    host_port=$OPTARG
                else
                    echo "invalid host_port format"
                    aws_socks_help
                    exit 1
                fi
            fi
            ;;

        # for help
        h|*)
            aws_socks_help
            exit 0
    esac
done

if [ ${use_local_web} -gt 0 ]; then
    local_proxydir="/Library/WebServer/Documents/"
else
    local_proxydir="/Applications/Safari.app/Contents/Resources"
fi

if [ "${MODE}" == "clear" ] || [ "${MODE}" == "normal" ]; then
    #if [ ${USE_SSH} -eq 0 ]; then
        kill_process "local.js"
        kill_process "watch_sso"
    #else
        remote_host=`echo ${host_port} | awk -F":" '{print $2}'`
        remote_ip=`get_dnsip ${remote_host}`

        kill_process $remote_ip
        kill_process $remote_host
        kill_process "ssh -D"
        kill_process "watch_socks"
    #fi

    stop_apache
    sudo networksetup -setautoproxystate ${ETH} off
    rm -f /tmp/shadowsocks*

    if [ ${restart} -gt 0 ]; then
        MODE="normal"
    fi
fi

if [ "${MODE}" == "normal" ]; then
    fill_and_run_proxy

    which curl >/dev/null
    t_res=$?
    if [ $t_res -eq 0 ]; then
        has_curl=1
    else
        has_curl=0
    fi

    sudo networksetup -setautoproxystate ${ETH} off

    update_pac $has_curl
    if [ $? -ne 0 ]; then
        exit 1
    fi

    if [ ! -f ${local_proxydir}/proxy.pac ]; then
        sudo networksetup -setautoproxyurl ${ETH} "http://david-stratusee.github.io/proxy.pac"
    else
        if [ ${use_local_web} -gt 0 ]; then
            start_apache
            sudo networksetup -setautoproxyurl ${ETH} "http://127.0.0.1/proxy.pac"
        else
            sudo networksetup -setautoproxyurl ${ETH} "file://localhost${local_proxydir}/proxy.pac"
        fi
    fi

    sudo networksetup -setautoproxystate ${ETH} on
fi
show_proxy


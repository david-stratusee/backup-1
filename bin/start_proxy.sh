#!/bin/bash -
#===============================================================================
#          FILE: start_proxy.sh
#         USAGE: ./start_proxy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月27日 23:39
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ${HOME}/bin/tools.sh

username=david
available_host_port=([0]="david:us.stratusee.com:2226" [1]="55e6658f0c1e66d617000070:shadowsocks-crazyman.rhcloud.com:22")
host_port=${available_host_port[1]}

aliveinterval=0
check_memory=0
SHADOW_DIR=${HOME}/work/openshift/shadowsocks
USE_SSH=0

function show_proxy_stat()
{
    echo ===========================
    echo "PROCESS INFO:"
    pss | grep -v grep | egrep --color=auto "(ssh -D|COMMAND|local.js|watch_socks|sslsplit|dnschef|watch_sso)"
    echo ===========================
    if [ ${USE_SSH} -ne 0 ] && [ -f /tmp/watch_socks.log ]; then
        echo "/tmp/watch_socks.log:"
        grep "ssh -D" /tmp/watch_socks.log
        echo ===========================
    elif [ ${USE_SSH} -eq 0 ]; then
        echo "LISTEN INFO:"
        #sudo netstat -anp | grep node
        sudo netstat -anp | grep 15500 | egrep --color=auto -i "(listen|established)"
        echo ===========================
        if [ -f /tmp/shadowsocks.log ]; then
            echo "/tmp/shadowsocks.log:"
            cat /tmp/shadowsocks.log
            echo ===========================
        fi
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
    pidc=`pss | grep -v "grep" | grep -c "$@"`
    if [ $pidc -gt 0 ]; then
        sshpid=`pss | grep -v "grep" | grep "$@" | awk '{print $2}'`
        for p in $sshpid; do
            echo "kill $@, pid: ${p}"
            sudo kill $p
        done
    fi
}

function kill_sslsplit()
{
    pidc=`pss | grep -v "proxychains" | grep -v "grep" | grep -c "sslsplit"`
    if [ $pidc -gt 0 ]; then
        sshpid=`pss | grep -v "proxychains" | grep -v "grep" | grep "sslsplit" | awk '{print $2}'`
        for p in $sshpid; do
            echo "kill sslsplit, pid: ${p}"
            sudo kill $p
        done
    fi
}

function kill_dnschef()
{
    pidc=`pss | grep -v "nohup" | grep -v "grep" | grep -c "dnschef.py"`
    if [ $pidc -gt 0 ]; then
        sshpid=`pss | grep -v "nohup" | grep -v "grep" | grep "dnschef.py" | awk '{print $2}'`
        for p in $sshpid; do
            echo "kill dnschef.py, pid: ${p}"
            sudo kill $p
        done
    fi
}

function clear_proxy()
{
    if [ ${USE_SSH} -eq 0 ]; then
        kill_process "local.js"
        kill_process "watch_sso"

        rm -f /tmp/shadowsocks.log
    else
        remote_host=`echo ${host_port} | awk -F":" '{print $2}'`
        remote_ip=`get_dnsip ${remote_host}`

        kill_process $remote_ip
        kill_process $remote_host
        kill_process "ssh -D"
        kill_process "watch_socks"
    fi

    kill_dnschef
    kill_sslsplit
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

function print_avail_host()
{
    nhost=0
    echo "available socks host:port is:[${#available_host_port[*]}]"
    for node in ${available_host_port[*]}; do
        if [ "${host_port}" == "${node}" ]; then
            echo " [${nhost}] $node [***]"
        else
            echo " [${nhost}] $node"
        fi

        nhost=`expr ${nhost} + 1`
    done
}

function start_proxy_help()
{
    echo "------------------------------------"
    echo "Help Usage: "

    echo -e "\n### for proxy   ###"
    echo "-c             : for clear socks proxy"
    echo "-l             : for query socks proxy"

    echo -e "\n### for profile ###"
    echo "-m             : check memory leak"

    echo -e "\n### for SSH     ###"
    echo "-s             : ssh mode"
    echo "-a NUM         : set ServerAliveInterval for sshtunnel, default 0, recommand 7200"
    echo "-p NUM|IP:PORT : set socks proxy's host_port"
    print_avail_host

    echo "no args for set proxy"
    echo "------------------------------------"
}

SET_MODE=0
SHOW_MODE=1
CLEAR_MODE=2

op_mode=${SET_MODE}
while getopts 'a:p:hclms' opt; do
    case $opt in
        # for proxy
        c)
            op_mode=${CLEAR_MODE}
            ;;
        l)
            op_mode=${SHOW_MODE}
            show_proxy_stat
            exit 0
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
                    start_proxy_help
                    exit 1
                fi
            fi
            ;;

        # for profile
        m)
            check_memory=1
            ;;

        # for help
        h|*)
            start_proxy_help
            exit 0
            ;;
    esac
done

if [ ${op_mode} -eq ${SHOW_MODE} ]; then
    show_proxy_stat
    exit 0
elif [ ${op_mode} -eq ${CLEAR_MODE} ]; then
    clear_proxy
    exit 0
fi

if [ $USE_SSH -ne 0 ]; then
    ssh_num=`pss | grep -v grep | grep -c "ssh -D"`
else
    ssh_num=`pss | grep -v grep | grep -c "local.js"`
fi
if [ ${ssh_num} -eq 0 ]; then
    clear_proxy

    #rm -f /tmp/proxy.pac
    #wget -nv http://david-holonetsecurity.github.io/proxy.pac -P /tmp/
    #sudo cp -f /tmp/proxy.pac /etc/polipo/proxy.pac

    fill_and_run_proxy
    #sudo /usr/local/bin/polipo logLevel=0xFF

    dnschef.sh
fi

kill_process "aie_watchdog"
kill_sslsplit
if [ -f /tmp/sslsplit.log ]; then
    sudo mv -f /tmp/sslsplit.log /tmp/sslsplit.log.bak
fi
#sudo mv -f /tmp/memtm_ssl.heap /tmp/memtm_ssl.heap.bak
sleep 1
script_prefix=
if [ $check_memory -ne 0 ]; then
    tcmalloc_path=`whereis libtcmalloc.so | awk '{for (i=1;i<=NF;i++) print $i}' | grep libtcmalloc.so`
    #script_prefix="LD_PRELOAD=${tcmalloc_path} HEAPCHECK=normal HEAPPROFILE=/tmp/memtm_ssl.heap PPROF_PATH=/usr/bin/pprof HEAP_CHECK_TEST_POINTER_ALIGNMENT=1 HEAP_CHECK_MAX_LEAKS=100"
    script_prefix="LD_PRELOAD=${tcmalloc_path} HEAPCHECK=normal PPROF_PATH=/usr/bin/pprof HEAP_CHECK_TEST_POINTER_ALIGNMENT=1 HEAP_CHECK_MAX_LEAKS=100"
fi
sudo ${script_prefix} /usr/bin/proxychains4 /usr/local/holonet/bin/sslsplit ssl 0.0.0.0 8443 tcp 0.0.0.0 8081 autossl 0.0.0.0 8082 1>/tmp/sslsplit.log 2>&1 &
sleep 1
sudo /usr/local/holonet/bin/aie_watchdog

echo "show state:"
sleep 1
show_proxy_stat

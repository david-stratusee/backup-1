#!/bin/bash -
#===============================================================================
#          FILE: aws.sh
#         USAGE: ./aws.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014/08/14 12:14
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
. ${HOME}/bin/tools.sh

function gohelp()
{
    echo -e "Usage: \n\t-m for module(b|c|u|g|p|s|aie|l2tp|...)\n\t-f for local file\n\t-r for remote_file\n\t-c for command\n\t-e for exit\n\e-l show session"
}

dsthost=""
username="david"
remotehome="/home/"$username"/"
dstip=""
ssh_dstport=""
scp_dstport=""
local_file=""
remote_file=""
cmd=""
do_exit=0
while getopts 'm:f:r:c:elh' opt; do
    case $opt in
        l)
            pss | grep -v grep | grep sockets
            exit 0
            ;;
        e)
            do_exit=1
            ;;
        m) 
            case $OPTARG in
                "c")
                    dsthost="aie.centos"
                    ;;
                "u")
                    dsthost="us.stratusee.com"
                    ssh_dstport=" -p 2226"
                    scp_dstport=" -P 2226"
                    ;;
                "g")
                    dsthost="github.com"
                    dstip="github.com"
                    username="git"
                    remotehome="/home/"$username"/"
                    ;;
                "b")
                    dsthost="aie.box"
                    username="stratusee"
                    remotehome="/home/"$username"/"
                    ;;
                "s")
                    dsthost="shadowsocks-crazyman.rhcloud.com"
                    username="55e6658f0c1e66d617000070"
                    remotehome="/var/lib/openshift/"$username"/"
                    ;;
                "p")
                    dsthost="php-crazyman.rhcloud.com"
                    username="5626ebbc2d5271dce200005b"
                    remotehome="/var/lib/openshift/"$username"/"
                    ;;
                *)
                    dsthost="dev-${OPTARG}.stratusee.com"
                    ;;
            esac
            ;;
        f)
            local_file=$OPTARG
            ;;
        r)
            remote_file=$OPTARG
            ;;
        c)
            cmd=$OPTARG
            ;;
        h|*)
            gohelp
            exit 0
    esac
done

if [ "${dsthost}" == "" ]; then
    echo "dsthost is none"
    gohelp
    exit 0
fi

if [ "${dstip}" == "" ]; then
    dstip=`get_dnsip ${dsthost}`
    echo "${dsthost} - ${dstip}"
fi

if [ ${do_exit} -ne 0 ]; then
    ssh -O stop ${username}@${dstip}${ssh_dstport}
    ps_count=`pss | grep -v grep | grep -c ${dstip}`
    if [ ${ps_count} -gt 0 ]; then
        sleep 1
        ps_count=`pss | grep -v grep | grep -c ${dstip}`
        if [ ${ps_count} -gt 0 ]; then
            ssh -O exit ${username}@${dstip}${ssh_dstport}
        fi
    fi
    exit 0
fi

if [ "${remote_file}" != "" ]; then
    start_dir=${remotehome}
    if [ "${remote_file:0:1}" == "/" ]; then
        start_dir=""
    fi

    if [ "${local_file}" != "" ]; then
        echo scp -r${scp_dstport} ${local_file} ${username}@${dstip}:${start_dir}${remote_file}
        scp -r${scp_dstport} ${local_file} ${username}@${dstip}:${start_dir}${remote_file}
    else
        echo scp -r${scp_dstport} ${username}@${dstip}:${start_dir}${remote_file} .
        scp -r${scp_dstport} ${username}@${dstip}:${start_dir}${remote_file} .
    fi
elif [ "${local_file}" != "" ]; then
    echo scp -r${scp_dstport} ${local_file} ${username}@${dstip}:${remotehome}
    scp -r${scp_dstport} ${local_file} ${username}@${dstip}:${remotehome}
else
    echo ssh${ssh_dstport} ${username}@${dstip} ${cmd}
    ssh${ssh_dstport} ${username}@${dstip} ${cmd}
fi

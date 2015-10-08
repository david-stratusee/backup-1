#!/bin/bash -
#===============================================================================
#          FILE: curl.sh
#         USAGE: ./curl.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/04/20 12:48
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cookie="__utmt=1; VT_PREFERRED_LANGUAGE=zh-cn; __utma=194538546.1343511174.1429341032.1429491971.1429502407.6; __utmb=194538546.14.10.1429502407; __utmc=194538546; __utmz=194538546.1429341183.2.2.utmcsr=aol|utmccn=(organic)|utmcmd=organic|utmctr=%22api.mcr.skype.com%22"
user_agent="Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.90 Safari/537.36"
#checkip="210.83.224.204"
addhead2="Accept-Language: zh-CN,zh;q=0.8,en;q=0.6,zh-TW;q=0.4"

hostfile=$1
inquire_resultfile=$2

function get_host_str()
{
    local inner_ip=$1
    echo $inner_ip
    echo $inner_ip >>${inquire_resultfile}
    url="https://www.virustotal.com/zh-cn/ip-address/${inner_ip}/information/"
    echo curl --ssl -k -b "${cookie}" -A "${user_agent}" -H "${addhead2}" $url
    curl --ssl -k -b "${cookie}" -A "${user_agent}" -H "${addhead2}" $url -o /tmp/sq.log
    echo $? >>${inquire_resultfile}
    grep "<a class=\"margin-left-1\" target=\"_blank\" href=" /tmp/sq.log | awk -F">" '{print $2}' | awk -F"<" '{print $1}' | sort -u >/tmp/sq1.log

    while read domain; do
        if [ -n "$domain" ]; then
            echo $domain
            echo $domain >>${inquire_resultfile}
            dig $domain | grep $inner_ip >>${inquire_resultfile}
        fi
    done </tmp/sq1.log;

    rm -f /tmp/sq.log /tmp/sq1.log
    echo "==========" >>${inquire_resultfile}
}

rm ${inquire_resultfile}
while read checkip; do
    echo $checkip
    get_host_str $checkip
done <${hostfile}

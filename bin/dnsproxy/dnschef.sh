#!/bin/bash -
#===============================================================================
#          FILE: dnschef.sh
#         USAGE: ./dnschef.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年08月25日 17:07
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function kill_dnschef_1()
{
    local val=0
    pidc=`ps -ef | grep -v "nohup" | grep -v "grep" | grep -c "dnschef.py"`
    if [ $pidc -gt 0 ]; then
        sshpid=`ps -ef | grep -v "nohup" | grep -v "grep" | grep "dnschef.py" | awk '{print $2}'`
        for p in $sshpid; do
            echo "kill dnschef.py, pid: ${p}"
            sudo kill $p
            val=1
        done
    fi

    return $val
}

function prepare_dns_list()
{
    if [ -f /tmp/proxy.pac ]; then
        rm -f /tmp/proxy.pac
    fi
    wget -T 10 -nv http://david-stratusee.github.io/proxy.pac.old -O /tmp/proxy.pac.old
    grep "1,$" /tmp/proxy.pac.old | grep -v "\/" | awk -F "\"" '{print $2}' >/tmp/whitelist.log

    grep "host.search" /tmp/proxy.pac.old | grep google | awk -F"/" '{print $2}' >> /tmp/whitelist.log

    echo ".box.com" >> /tmp/whitelist.log
    echo "boxcloud.com" >> /tmp/whitelist.log

    sed -i -e 's/\./\\\./g' /tmp/whitelist.log
}

echo pkill dnschef.py
kill_dnschef_1
result=$?

if [ $# -gt 0 ] && [ "$1" == "-c" ]; then
    exit 0
fi

if [ $result -gt 0 ]; then
    sleep 1
fi

localdns=`grep nameserver /etc/resolv.conf | awk '{print $2"#53,"}'`
localdns=`echo $localdns | sed -e 's/ //g'`
#echo $localdns

prepare_dns_list

echo start dnschef.py
echo ${HOME}/bin/dnschef.py --file ${HOME}/bin/dnschef.ini --dnsfile /tmp/whitelist.log --logfile /tmp/dnschef.log --nameservers ${localdns}208.67.220.220#53#tcp,208.67.222.222#53#tcp,209.244.0.3#53#tcp,209.244.0.4#53#tcp -i 0.0.0.0 -q
sudo nohup ${HOME}/bin/dnschef.py --file ${HOME}/bin/dnschef.ini --dnsfile /tmp/whitelist.log --logfile /tmp/dnschef.log --nameservers ${localdns}208.67.220.220#53#tcp,208.67.222.222#53#tcp,209.244.0.3#53#tcp,209.244.0.4#53#tcp -i 0.0.0.0 -q 1>/dev/null 2>&1 &
#sudo nohup ${HOME}/bin/dnschef.py --file ${HOME}/bin/dnschef.ini --dnsfile /tmp/whitelist.log --nameservers ${localdns}208.67.220.220#53#tcp,208.67.222.222#53#tcp,209.244.0.3#53#tcp,209.244.0.4#53#tcp -i 0.0.0.0 -q 1>/dev/null 2>&1 &
sleep 1
echo show result:

ps axf | grep dnschef

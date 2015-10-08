#!/bin/bash -
#===============================================================================
#          FILE: a.sh
#         USAGE: ./a.sh
#        AUTHOR: dengwei, david@holonetsecurity.com
#       CREATED: 2015年09月15日 10:51
#===============================================================================

set -o nounset                              # Treat unset variables as an error

. ~/bin/tools.sh

function ip2num()
{
    IP_ADDR=$1 
    #echo $IP_ADDR
    #[[ "$IP_ADDR" =~ "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" ]] || { echo "ip format error."; exit 1; }   
    IP_LIST=${IP_ADDR//./ }; 
    read -a IP_ARRAY <<<${IP_LIST};   
    num=$(( ${IP_ARRAY[0]}<<24 | ${IP_ARRAY[1]}<<16 | ${IP_ARRAY[2]}<<8 | ${IP_ARRAY[3]} )); 
    echo $num
}

function mask2str()
{
    l_smask=$1
    let new_smask=l_smask-1
    N=$((0xffffffff - ${new_smask}))
    H1=$(($N & 0x000000ff))
    H2=$((($N & 0x0000ff00) >> 8))
    L1=$((($N & 0x00ff0000) >> 16))
    L2=$((($N & 0xff000000) >> 24))
    echo "$L2.$L1.$H2.$H1"
}

apnic_file="delegated-apnic-latest"
apnic_url="http://ftp.apnic.net/apnic/stats/apnic/${apnic_file}"

if [ ! -f ${apnic_file} ]; then
    wget -nv ${apnic_url}
    if [ $? -ne 0 ]; then
        exit 1
    fi
fi

grep -i "|cn|" delegated-apnic-latest | grep apnic | grep ipv4 >/tmp/apnic_file

last_sip=0
last_sip_str=""
last_smask=0
last_sip_dest=0

while read line; do
    sip_str=`echo $line | awk -F"|" '{print $4}'`
    smask=`echo $line | awk -F"|" '{print $5}'`

    sip=`ip2num $sip_str`
    let sip_dest=sip+smask

    if [ $last_sip_dest -eq 0 ]; then
        last_sip_dest=$sip_dest
        last_sip_str=$sip_str
        last_smask=$smask
        last_sip=$sip
    elif [ $last_sip_dest -eq $sip ]; then
        last_sip_dest=$sip_dest
        let last_smask+=smask
    else
        echo "localnet "$last_sip_str/`mask2str $last_smask`
        # $last_sip $last_smask $last_sip_dest
        last_sip_dest=$sip_dest
        last_sip_str=$sip_str
        last_smask=$smask
        last_sip=$sip
    fi
done </tmp/apnic_file

if [ $last_sip_dest -ne 0 ]; then
    echo "localnet "$last_sip_str/`mask2str $last_smask`
    #$last_sip $last_smask $last_sip_dest
fi

rm -f /tmp/apnic_file 
error_echo "rm -f delegated-apnic-latest"

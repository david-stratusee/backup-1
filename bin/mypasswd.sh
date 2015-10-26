#!/bin/bash -
#===============================================================================
#          FILE: mypasswd.sh
#         USAGE: ./mypasswd.sh
#        AUTHOR: dengwei (), david@stratusee.com
#  ORGANIZATION: 
#       CREATED: 2015/09/15 10:15
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function upper_echo()
{
	src="$1"
	echo -e "$src" | awk '{print toupper($0)}'
}

if [ $# -lt 2 ]; then
    echo "`basename $0` domain username"
    exit 1
fi

system=`uname -s`

domain=$1
username=$2
domlen=${#domain}
tmpfile=${HOME}/.mypasswd

if [ -f ${tmpfile} ]; then
    nitem=$(grep -c "${domain} ${username}" ${tmpfile})
    if [ $nitem -eq 1 ]; then
        echo $(grep "${domain} ${username}" ${tmpfile} | awk '{print $3}')
        exit 0
    elif [ $nitem -gt 1 ]; then
        echo "More than one item is recorded in ${tmpfile}"
        grep "${domain} ${username}" ${tmpfile} | awk '{print $1" "$2}'
        exit 1
    fi
fi

all_str=${domain}.${username}.${domlen}

#echo $all_str
md5_ret=`mymd5 $all_str`
#echo $md5_ret

domlen=6
let ret2_start=domlen+1
let ret2_len=16-ret2_start
ret1=${md5_ret:0:${domlen}} 
ret2=${md5_ret:${ret2_start}:${ret2_len}} 

password="$ret1@`upper_echo $ret2`"
echo $password
echo "${domain} ${username} ${password}" >>${tmpfile}

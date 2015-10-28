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

function pass_help()
{
    main_name=`basename $1`
    echo "${main_name} domain username"
    echo "${main_name} -e for enc_file"
    echo "${main_name} -d for dec_file"
}

enc_key=david
tmpfile=${HOME}/.mypasswd
function dec_file()
{
    ccrypt -d -K ${enc_key} -q $1.cpt
}

function enc_file()
{
    ccrypt -e -K ${enc_key} -q $1
}

which ccrypt >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo We should install ccrypt to check password
    exit 1
fi

if [ $# -eq 1 ]; then
    if [ "$1" == "-d" ]; then
        dec_file ${tmpfile}
        echo check ${tmpfile}
        exit 0
    elif [ "$1" == "-e" ]; then
        enc_file ${tmpfile}
        echo check ${tmpfile}.cpt
        exit 0
    fi

    pass_help $0
    exit 1
fi

if [ $# -lt 2 ]; then
    pass_help $0
    exit 1
fi
domain=$1
username=$2
domlen=${#domain}

if [ -f ${tmpfile}.cpt ]; then
    dec_file ${tmpfile}
    if [ $? -ne 0 ]; then
        rm -f ${tmpfile}.cpt
        exit 1
    fi

    nitem=$(grep -c "${domain} ${username}" ${tmpfile})
    if [ $nitem -eq 1 ]; then
        echo $(grep "${domain} ${username}" ${tmpfile} | awk '{print $3}')
        enc_file ${tmpfile}
        exit 0
    elif [ $nitem -gt 1 ]; then
        echo "More than one item is recorded in ${tmpfile}"
        grep "${domain} ${username}" ${tmpfile} | awk '{print $1" "$2}'
        enc_file ${tmpfile}
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
enc_file ${tmpfile}

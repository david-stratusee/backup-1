#!/bin/bash -
#===============================================================================
#          FILE:  tools.sh
#   DESCRIPTION:  工具函数
# 
#        AUTHOR:  dengwei dengwei@venus.com
#       VERSION:  1.0
#       CREATED:  2011年09月01日 15时21分44秒 CST
#===============================================================================

#-------------------------------------------------------------------------------
#   colorecho
#-------------------------------------------------------------------------------
<<BLOCK
red="\e[0;31m"
RED="\e[1;31m"
blue="\e[0;34m"
BLUE="\e[1;34m"
cyan="\e[0;36m"
CYAN="\e[1;36m"
green="\e[0;32m"
yellow="\e[0;33m"
magenta="\e[0;35m"
white="\e[0;37m"
#30=black 31=red 32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white
NC="\e[0m" # No Color
# --> Nice. Has the same effect as using "ansi.sys" in DOS
BLOCK

function colorecho()
{
    color_print.py $1 "$2"
    echo
#	color=$1
#	shift
#	echo -e "$color$@${NC}"
}

function colornecho()
{
    color_print.py $1 "$2"
#	color=$1
#	shift
#	echo -ne "$color$@${NC}"
}

function error_echo()
{
    >&2 color_print.py red "$@"
}

#-------------------------------------------------------------------------------
#   Check if $pid (could be plural) are running
#-------------------------------------------------------------------------------
function checkpid()
{
	local i

	for i in $* ; do
		[ -d "/proc/$i" ] && return 0
	done
	return 1
}

#-------------------------------------------------------------------------------
#   Output PIDs of matching processes, found using pidof
#-------------------------------------------------------------------------------
function __pids_pidof()
{
	pidof -c -o $$ -o $PPID -o %PPID -x "$1" || \
		pidof -c -o $$ -o $PPID -o %PPID -x "${1##*/}"
}

#-------------------------------------------------------------------------------
#   echo as upper char
#-------------------------------------------------------------------------------
function upper_echo()
{
	src="$1"
	echo -e "$src" | awk '{print toupper($0)}'
}

#-------------------------------------------------------------------------------
#   echo as lower char
#-------------------------------------------------------------------------------
function lower_echo()
{
	src="$1"
	echo -e "$src" | awk '{print tolower($0)}'
}

#-------------------------------------------------------------------------------
#   echo to file
#	file_echo filename "xxxxx"
#-------------------------------------------------------------------------------
function file_echo()
{
	filename="$1"
	shift
	echo -e "$@" >> $filename
}

#-------------------------------------------------------------------------------
#   execute 
#	execute [-f filename] cmd args...
#-------------------------------------------------------------------------------
function execute_help()
{
	echo "execute [-f filename] cmd args..."
}

# usage: "execute [-f filename] cmd args..."
function execute()
{
	ret_val=1
	if [ $# -eq 0 ]; then
		execute_help
	else
		filename=""
		if [ "$1" = "-f" ]; then
			if [ $# -lt 3 ]; then
				execute_help
				return $ret_val;
			fi
			filename=$2
			shift
			shift
		fi

		if [ "$filename" != "" ]; then
			echo -n "execute: $@"
			echo "[`date +%H:%M:%S`] $@" >> $filename
			$@ >> $filename
			ret_val=$?

			echo -n "[`date +%H:%M:%S`] execute " >> $filename
			if [ $ret_val -eq 0 ]; then
				colorecho green "	--- [OK]"
			else
				colorecho red "	--- [FAIL]"
			fi
		else
			echo "[`date +%H:%M:%S`] $@"
			$@
			ret_val=$?

			echo -n "[`date +%H:%M:%S`] execute "
			if [ $ret_val -eq 0 ]; then
				colorecho green "	--- [OK]"
			else
				colorecho red "	--- [FAIL]"
			fi
		fi
	fi

	return $ret_val
}

function execute_hint()
{
	ret_val=1
	if [ $# -eq 0 ]; then
		execute_help
	else
		filename=""
		if [ "$1" = "-f" ]; then
			if [ $# -lt 3 ]; then
				execute_help
				return $ret_val;
			fi
			filename=$2
			shift
			shift
		fi

		if [ "$filename" != "" ]; then
			echo -n "[`date +%H:%M:%S`] $@"
			echo "[`date +%H:%M:%S`] execute: $@" >> $filename
			$@ >> $filename
			ret_val=$?
		else
			echo -n "[`date +%H:%M:%S`] $@"
			$@
			ret_val=$?
		fi

		if [ $ret_val == 0 ]; then
			colorecho green "	--- [OK]"
		else
			colorecho red "	--- [FAIL]"
		fi
	fi

	return $ret_val
}

function check_process_num()
{
    ps_count=`ps -ef | grep -v "bash\b" | grep "$1\b" | grep -v grep | wc -l`
    return $ps_count
}

function time_echo()
{
    echo -e "["`date +'%H:%M:%S'`"] $@"
}

function title_echo()
{
    level=$1
    shift
    if [ $level -eq 0 ]; then
        echo -e "\n ***** [`date +'%m-%d %H:%M:%S'`] $@ *****"
    else
        echo -e "\n ----- [`date +'%m-%d %H:%M:%S'`] $@ -----"
    fi
}

function get_dnsip()
{
    domain=$1
    dns_server="208.67.220.220"
    if [ $# -gt 1 ]; then
        dns_server=$2
    fi
    dstip=`grep ${domain} /etc/hosts | awk '{print $1}'`
    if [ "$dstip" == "" ]; then
        dstip=`dig +noquestion +nocomment +tcp @${dns_server} ${domain} | grep "\bA\b" | awk '{print $5}'`
        if [ "$dstip" == "" ]; then
            echo $domain
            return 0
        fi
    fi

    echo $dstip
    return 0
}

if [ -d /proc ]; then
    alias pss='ps axfo user,pid,ppid,pcpu,pmem,rss,nlwp,psr,stat,start_time,etime,wchan:18,command'
else
    alias pss='ps axo user,pid,ppid,pcpu,pmem,rss,stat,stime,etime,command'
fi

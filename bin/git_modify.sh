#!/bin/bash -
#===============================================================================
#          FILE: git_modify.sh
#         USAGE: ./git_modify.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 07/27/2014 16:35
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function select_key()
{
    sel_key=$1
    column_count=`git status | grep "$sel_key" | awk '{print NF}' | sort | uniq`
    if [ "$column_count" == "" ]; then
        return
    fi
    if [ $# -eq 1 ]; then
        if [ $column_count -eq 3 ]; then
            git status | awk '{if (NF==3 && index($2, modl_key)==1) print $3}' modl_key="$sel_key"
        else
            git status | awk '{if (NF==2 && index($1, modl_key)==1) print $2}' modl_key="$sel_key"
        fi
    else
        if [ $column_count -eq 3 ]; then
            git status | awk '{if (NF==3 && index($2, modl_key)==1) print $3}' modl_key="$sel_key" >>$2
        else
            git status | awk '{if (NF==2 && index($1, modl_key)==1) print $2}' modl_key="$sel_key" >>$2
        fi
    fi
}

logfile=""
need_log=false

show_modify=true
show_delete=true

while getopts 'f:mdh' opt; do
    case $opt in
        f) 
            logfile=$OPTARG
            need_log=true
            ;;
        m)
            show_delete=false
            ;;
        d)
            show_modify=false
            ;;
        h|*)
            echo "Usage: `basename $0` [-f logfile] [-m] [-d]"
            echo "       -h for help"
            exit 0
            ;;
    esac
done

if [ ! -d .git ] && [ ! -d ../.git ] && [ ! -d ../../.git ] && [ ! -d ../../../.git ] && [ ! -d ../../../../.git ] && [ ! -d ../../../../../.git ]; then
    echo "You are not in one git repository"
    exit 0
fi

if [ $need_log == true ]; then
    echo get logfile: $logfile
fi

count=`echo $LANG | grep -i "zh_cn" | wc -l`
if [ $count -eq 0 ]; then
    modi_key="modified:"
    del_key="deleted:"
else
    modi_key="修改："
    del_key="删除："
fi

if [ $need_log == true ]; then
    echo -n "" >$logfile
fi

# get modified
if [ $show_modify == true ]; then
    echo
    color_print.py red "------------ $modi_key ------------"
    echo
    select_key $modi_key
    if [ $need_log == true ]; then
        select_key $modi_key $logfile
    fi
fi

if [ $show_delete == true ]; then
    echo
    color_print.py red "------------ $del_key ------------"
    echo
    select_key $del_key
    if [ $need_log == true ]; then
        echo >>$logfile
        select_key $del_key $logfile
    fi
fi


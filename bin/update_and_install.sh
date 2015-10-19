#!/bin/bash -
#===============================================================================
#          FILE: update_and_install.sh
#         USAGE: ./update_and_install.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/06/18 10:25
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function git_pull()
{
    out=`git pull`
    result=$?

    rm -f ~/.netrc
    if [ $result -ne 0 ]; then
        echo "error when git pull"
        return 0
    else
        echo "===================="
        echo $out
        echo "===================="

        result=`echo $out | grep -c "Already up-to-date"`
        if [ $result -ne 0 ]; then
            return 0
        else
            return 1
        fi
    fi
}

if [ ! -f ~/.netrc ]; then
    echo "netrc does not exist, exit..."
    exit 1
fi

curdir=`pwd`
cd ${HOME}/work/AIE
git_pull
if [ $? -ne 0 ]; then
    ./aie_compile_startup.sh
else
    echo "no need to compile"
fi
cd ${curdir}

/usr/local/holonet/scripts/show_core.sh

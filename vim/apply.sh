#!/bin/bash -
#===============================================================================
#          FILE: apply.sh
#         USAGE: ./apply.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年07月19日 00:54
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

cp vimrc ~/.vimrc

if [ -d ~/.vim ]; then
    mv ~/.vim ~/vim_bak
fi
mkdir ~/.vim
cp -r bundle ~/.vim/

curdir=`pwd`
while read line; do
    echo ===================================
    echo apply -- $line

    cd ~/.vim/bundle/
    url=`echo $line | awk '{print $2}'`
    name=`echo $line | awk '{print $1}'`
    rm -rf $name
    git clone ${url} --depth 1
    cd ${curdir}

    ls ./git_save/${name} 1>/dev/null 2>/dev/null
    if [ $? -eq 0 ]; then
        echo ------------- update local files
        cp -r ./git_save/${name}/* ~/.vim/bundle/${name}
    fi
    echo ===================================
done <./bundle.list

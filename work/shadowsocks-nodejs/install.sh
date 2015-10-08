#!/bin/bash -
#===============================================================================
#          FILE: a.sh
#         USAGE: ./a.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015/09/02 10:26
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function show_name()
{
    echo $@
    $@
}

name="shadowsocks"
show_name rhc app delete shadowsocks
show_name rm -rf shadowsocks
show_name rhc app create ${name} nodejs-0.10 --from-code https://github.com/mrluanma/shadowsocks-heroku.git
show_name rhc show-app ${name}

filelist=`ls | grep -v install.sh`

show_name cp -f $filelist $name

cd $name
npm install
cd ..

show_name rhc app restart -a $name

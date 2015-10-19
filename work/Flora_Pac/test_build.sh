#!/bin/sh

if [ ! -f delegated-apnic-latest ]; then
    wget http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest
    if [ $? -ne 0 ]; then
        exit 1
    fi
fi

./flora_pac -x "SOCKS5 127.0.0.1:15500; DIRECT" -c delegated-apnic-latest
#rm -f delegated-apnic-latest

if [ $# -gt 0 ]; then
    dest_path=$1
else
    dest_path="../../../david-holonetsecurity.github.io"
fi
if [ -f /Library/WebServer/Documents/proxy.pac ]; then
    cp -f flora_pac.pac /Library/WebServer/Documents/proxy.pac
fi
if [ -d $dest_path ]; then
    echo cp ./flora_pac.pac ${dest_path}/proxy.pac
    mv -f ./flora_pac.pac ${dest_path}/proxy.pac
else
    echo "argument must be one directory path"
    rm -f flora_pac.pac
    exit 0
fi

cd $dest_path
pwd
echo git_commit.sh -a add -m "fix" proxy.pac
git_commit.sh -a add -m "fix" proxy.pac
echo git push
git push

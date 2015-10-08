#!/bin/bash -
#===============================================================================
#          FILE: set_proxy.sh
#         USAGE: ./set_proxy.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014/12/14 19:49
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

function show_proxy()
{
    echo ===========================
    echo "http_proxy state:"
    networksetup -getwebproxy Wi-Fi
    echo ===========================
    echo "https_proxy state:"
    networksetup -getsecurewebproxy Wi-Fi
    echo ===========================
    echo "socks_proxy state:"
    networksetup -getsocksfirewallproxy Wi-Fi
    echo ===========================
    echo "pac_proxy state:"
    networksetup -getautoproxyurl wi-fi
    echo ===========================
}

if [ $# -gt 0 ] && [ "$1" == "-h" ]; then
    echo "Usage: `basename $0` [-s|-p|-sp|-l] IP"
    echo "-s for adding https proxy, -p for adding http proxy, -sp for adding http and https proxy"
    echo "default is only http proxy, as -p"
    exit 0
fi

http_proxy=1
https_proxy=0
if [ $# -gt 0 ]; then
    if [ "$1" == "-l" ]; then
        show_proxy
        exit 0
    fi

    if [ "$1" == "-s" ]; then
        http_proxy=0
        https_proxy=1
        shift
    elif [ "$1" == "-p" ]; then
        http_proxy=1
        https_proxy=0
        shift
    elif [ "$1" == "-sp" ]; then
        http_proxy=1
        https_proxy=1
        shift
    elif [ "$1" == "-c" ]; then
        http_proxy=0
        https_proxy=0
        shift
    fi
fi

if [ $# -gt 0 ]; then
    sudo networksetup -setwebproxy Wi-Fi $1 3128
    sudo networksetup -setsecurewebproxy Wi-Fi $1 3128
fi

if [ ${http_proxy} -gt 0 ]; then
    sudo networksetup -setwebproxystate Wi-Fi on
else
    sudo networksetup -setwebproxystate Wi-Fi off
fi
if [ ${https_proxy} -gt 0 ]; then
    sudo networksetup -setsecurewebproxystate Wi-Fi on
else
    sudo networksetup -setsecurewebproxystate Wi-Fi off
fi

if [ ${http_proxy} -gt 0 ] || [ ${https_proxy} -gt 0 ]; then
    sudo networksetup -setsocksfirewallproxystate Wi-Fi off
fi

show_proxy

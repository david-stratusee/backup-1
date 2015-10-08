#!/bin/bash -
#===============================================================================
#          FILE: tshark.sh
#         USAGE: ./tshark.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014/12/12 10:15
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

if [ $# -eq 0 ] || [ "$1" == "-h" ]; then
    echo "for icap: wshark.sh \"tcp port 1344\" -i lo"
    echo "for http: wshark.sh \"tcp port 3128\" -i enp0s3"
    exit 0
fi

filter=$1
shift

echo tshark -O \"http,message-http,icap,data-text-lines\" -d \"tcp.port==8080,http\" -d \"tcp.port==3128,http\" -d \"tcp.port==1344,icap\" -f \"$filter and greater 80\" $@
sudo tshark -O "http,message-http,icap,data-text-lines" -d "tcp.port==8080,http" -d "tcp.port==3128,http" -d "tcp.port==1344,icap" -f "$filter and greater 80" $@

#!/bin/bash -
#===============================================================================
#          FILE: start_sslsplit1.sh
#         USAGE: ./start_sslsplit1.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年04月23日 01:19
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sudo sysctl -q -w net.ipv4.ip_forward=1

TRANS_PORT=8081
SSL_PORT=8443

echo =================================
sudo iptables -t nat -F
sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-ports ${TRANS_PORT}
sudo iptables -t nat -A PREROUTING -p tcp --dport 443 -j REDIRECT --to-ports ${SSL_PORT}
sudo iptables -t nat -A PREROUTING -p tcp --dport 587 -j REDIRECT --to-ports ${SSL_PORT}
sudo iptables -t nat -A PREROUTING -p tcp --dport 465 -j REDIRECT --to-ports ${SSL_PORT}
sudo iptables -t nat -A PREROUTING -p tcp --dport 993 -j REDIRECT --to-ports ${SSL_PORT}
sudo iptables -t nat -A PREROUTING -p tcp --dport 5222 -j REDIRECT --to-ports ${TRANS_PORT}
sudo iptables -t nat -A POSTROUTING -j MASQUERADE
sudo iptables -t nat -L
echo =================================

rm -rf ./log_data
mkdir ./log_data

debug_mode=""
if [ $# -gt 0 ] && [ "$1" == "-D" ]; then
    debug_mode="-D"
    shift
fi
CERT_DIR="/home/david/work/keys/squid_cert"
ARGUMENTS="-P -l ./log_data/connections.log -S ./log_data/ ${debug_mode} -f ssee_svc"
PEM_ARGS="-k ${CERT_DIR}/holonet.key -c ${CERT_DIR}/holonet.pem -C ${CERT_DIR}/holonet_ca.pem"
#PEM_ARGS="-k ${CERT_DIR}/holonet.key -c ${CERT_DIR}/holonet_all.pem"

echo ./sslsplit ${ARGUMENTS} ${PEM_ARGS} ssl 0.0.0.0 ${SSL_PORT} tcp 0.0.0.0 ${TRANS_PORT}
echo -----------
if [ $# -eq 0 ]; then
    sudo ./sslsplit ${ARGUMENTS} ${PEM_ARGS} ssl 0.0.0.0 ${SSL_PORT} tcp 0.0.0.0 ${TRANS_PORT}
else
    sudo ./sslsplit ${ARGUMENTS} ${PEM_ARGS} ssl 0.0.0.0 ${SSL_PORT} tcp 0.0.0.0 ${TRANS_PORT} >$1 2>&1
fi

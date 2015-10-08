#!/bin/bash -
#===============================================================================
#          FILE: make_ssldb.sh
#         USAGE: ./make_ssldb.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 11/06/2014 09:57
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

sudo rm -rf /usr/local/squid/var/lib/ssl_db
sudo /usr/local/squid/libexec/ssl_crtd -c -s /usr/local/squid/var/lib/ssl_db
sudo chown -R proxy.proxy /usr/local/squid/var/lib/ssl_db

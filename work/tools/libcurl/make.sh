#!/bin/bash -
#===============================================================================
#          FILE: make.sh
#         USAGE: ./make.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月12日 03:02
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

./aie_patch.sh -f curl.patch -t curl-7.40.0 -u "http://curl.haxx.se/download/curl-7.40.0.tar.gz"

cd curl-7.40.0
export LIBS=-ljemalloc && ./configure --prefix=/usr --libdir=/usr/lib64 --disable-symbol-hiding
if [ $? -ne 0 ]; then
    echo error when configure
    exit 1
fi

make
if [ $? -ne 0 ]; then
    echo error when make
    exit 1
fi

sudo make install
if [ $? -ne 0 ]; then
    echo error when make install
    exit 1
fi

cd ..

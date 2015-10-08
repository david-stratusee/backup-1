#!/bin/bash -
#===============================================================================
#          FILE: test_gprof.sh
#         USAGE: ./test_gprof.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2015年02月05日 01:22
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

if [ ! -f ./gprof/gprof-helper.so ]; then
    cd gprof
    make -f makefile_gprof
    cd ..
fi

cbuild.sh -a clean
cbuild.sh -q -g "TYPES=gprof"
time LD_PRELOAD=./gprof/gprof-helper.so ./multi_test $@
cbuild.sh -a clean
cbuild.sh -q

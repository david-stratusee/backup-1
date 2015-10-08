#!/bin/bash -

ulimit -c unlimited
echo >result/result_400_direct.log
./multi_test -f data/ds_100k.txt -a 400 -d direct -t 600 -o result/aie_10min_direct.csv >>result/result_400_direct.log 2>&1
sleep 30
./multi_test -s -f data/ds_100k.txt -a 400 -d s_direct -t 600 -o result/aie_10min_direct.csv >>result/result_400_direct.log 2>&1

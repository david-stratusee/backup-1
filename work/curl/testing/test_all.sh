#!/bin/bash -

sudo rm -rf ../result/*
ulimit -c unlimited

./compare21.sh -a 400 -d aie -t 300 -o ../result/aie_5min_400.csv >../result/result_400.log 2>&1
sleep 60
./compare21.sh -a 300 -d aie -t 300 -o ../result/aie_5min_300.csv >../result/result_300.log 2>&1
sleep 60
./compare21.sh -a 500 -d aie -t 300 -o ../result/aie_5min_500.csv >../result/result_500.log 2>&1

sleep 60

./compare21.sh -a 400 -d aie -t 300 -b 1 -o ../result/aie_5min_400_b1.csv >../result/result_400_b1.log 2>&1
sleep 60
./compare21.sh -a 300 -d aie -t 300 -b 1 -o ../result/aie_5min_300_b1.csv >../result/result_300_b1.log 2>&1
sleep 60
./compare21.sh -a 500 -d aie -t 300 -b 1 -o ../result/aie_5min_500_b1.csv >../result/result_500_b1.log 2>&1

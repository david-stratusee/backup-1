#!/bin/bash -

args=""
while getopts 'o:d:r:b:q:a:n:t:f:hs' opt; do
    case $opt in
        d)
            desc=$OPTARG
            ;;
        h)
            ./multi_test -h
            exit 0
            ;;
        *)
            args=${args}"-$opt $OPTARG "
            ;;
    esac
done

echo ${args}
echo ${desc}

ulimit -c unlimited

./add_route_2.sh del

./add_route_3.sh
./test.sh $args -d "2c_"${desc}
./add_route_3.sh del

./test.sh $args -d "direct"

./add_route_2.sh
./test.sh $args -d "1c_"${desc}
./add_route_2.sh del

./add_route_3.sh
./test.sh $args -s -d "s_2c_"${desc}
./add_route_3.sh del

./test.sh $args -d "s_direct" -s

./add_route_2.sh
./test.sh $args -s -d "s_1c_"${desc}
./add_route_2.sh del


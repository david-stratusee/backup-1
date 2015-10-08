#!/bin/bash -

#IP=192.168.10.31
IP=192.168.10.102
AIE_IP=192.168.0.103

if [ $# -gt 0 ]; then
	echo del
	sudo route del -host ${IP}
else
	sudo route add -host ${IP} gw ${AIE_IP} dev eth0
fi

ip route

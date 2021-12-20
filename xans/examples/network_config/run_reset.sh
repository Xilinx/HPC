#!/usr/bin/env bash

#usage: ./run_reset.sh ./config.txt

while IFS=' ' read -r hostname ipAddr xclbin devId
do
    ssh -f $hostname  "yes y | /opt/xilinx/xrt/bin/xbutil reset -d $devId"
    echo "reset for $hostname $devId"
done <"$1"

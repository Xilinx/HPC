#!/usr/bin/env bash

CUR_DIR=$(pwd)

while IFS=' ' read -r hostname ipAddr xclbin devId
do
    msg="hostname: $hostname, ipAddr: $ipAddr, xclbin: $xclbin, devID: $devId"
    echo "$msg"
    ssh -f $hostname  "cd $CUR_DIR;  bash netConfig.sh $hostname $xclbin $devId $2 $ipAddr"
    sleep 1
done <"$1"

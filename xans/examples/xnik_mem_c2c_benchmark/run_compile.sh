#!/usr/bin/env bash

CUR_DIR=$(pwd)

ip_file="./ip.txt"
rm -f $ip_file

while IFS=' ' read -r hostname ipAddr xclbin devId
do
    msg="hostname: $hostname, ipAddr: $ipAddr, xclbin: $xclbin, devID: $devId"
    echo "$msg"
    echo "$ipAddr" >> $ip_file
    ssh -f $hostname  "cd $CUR_DIR; bash compile.sh $hostname"
    sleep 1
done <"$1"

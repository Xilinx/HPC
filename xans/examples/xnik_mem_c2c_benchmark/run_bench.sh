#!/usr/bin/env bash

# Copyright 2021-2022 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CUR_DIR=$(pwd)
rm -rf log*
rm -rf xnik_benchmark.csv

size_list=()
while read size
do
    size_list+=($size)
done < $2

ip_file="./ip.txt"
n=0

for i in "${size_list[@]}"
do
    echo "${size_list[n]}"
    tac $1 | while IFS=' ' read -r hostname ipAddr xclbin devId
    do
        socket_file="./${hostname}_${devId}_sockets.txt"
        ssh -f $hostname  "cd $CUR_DIR; bash command.sh $hostname $socket_file $ip_file ${size_list[n]} $3" 
        sleep 2
    done    
    let n=n+1
done

sleep 5

egrep  -h ^DATA_CSV log* | grep Latency | head -1 > xnik_benchmark.csv
egrep  -h ^DATA_CSV log* | grep -v Latency >> xnik_benchmark.csv

sort -k 2 -n -o xnik_benchmark.csv xnik_benchmark.csv

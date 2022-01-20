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

matrix_list=()
while read matrix
do
    matrix_list+=($matrix)
done < $2

ip_file="./ip.txt"
n=0



for i in "${matrix_list[@]}"
do
    echo "${matrix_list[n]}"
    c=0
    tac $1 | while IFS=' ' read -r hostname ipAddr xclbin devId
    do
        socket_file="./${hostname}_${devId}_sockets.txt"

        if [[ $c = 1 ]]; then
            ssh -f $hostname  "cd $CUR_DIR; bash command.sh $hostname $socket_file $ip_file ./sig_dat ./vec_dat ${matrix_list[n]}" 
            sleep 3 
        else
            ssh -f $hostname  "cd $CUR_DIR; bash command.sh $hostname $socket_file $ip_file ./vec_dat ${matrix_list[n]}" 
            sleep 3
        fi
        let "c=c+1"
    done    
    let n=n+1
done


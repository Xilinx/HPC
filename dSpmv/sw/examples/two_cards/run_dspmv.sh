#!/usr/bin/env bash

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


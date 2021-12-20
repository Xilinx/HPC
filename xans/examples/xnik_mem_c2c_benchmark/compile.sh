#!/bin/bash

CUR_DIR=$(pwd)

cd $CUR_DIR/../network_config

echo "compile network_config"
make clean TARGET=hw HOST=$1
make host TARGET=hw HOST=$1

cd $CUR_DIR
make clean TARGET=hw HOST=$1
make host TARGET=hw HOST=$1

sleep 1
echo "done for $1"

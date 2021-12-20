#!/bin/bash

echo "./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/config.exe $2 $3 $4 $5"
make clean TARGET=hw HOST=$1
make host TARGET=hw HOST=$1
./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/config.exe $2 $3 $4 $5
sleep 1
echo "done for $1"

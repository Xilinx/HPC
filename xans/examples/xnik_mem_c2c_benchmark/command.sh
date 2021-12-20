#!/bin/bash

echo "./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/c2c_benchmark.exe $2 $3 $4 $5"
./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/c2c_benchmark.exe $2 $3 $4 $5 | tee log-$4.txt

echo "done for $HOST"

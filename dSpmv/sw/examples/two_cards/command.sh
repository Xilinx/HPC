#!/bin/bash

if [ "$#" -eq 5 ]; then
    echo 5
    echo "./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/dspmv.exe $2 $3 $4 $5"
    ./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/dspmv.exe $2 $3 $4 $5
fi

if [ "$#" -eq 6 ]; then
    echo 6
    echo "./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/dspmv.exe  $2 $3 $4 $5 $6"
    ./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/dspmv.exe $2 $3 $4 $5 $6
fi


echo "done for $HOST"

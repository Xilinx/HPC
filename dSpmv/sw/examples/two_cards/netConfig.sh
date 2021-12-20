#!/bin/bash

echo "../../../../xans/examples/network_config/build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/config.exe $2 $3 $4 $5"
../../../../xans/internal_demos/network_config/build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/$1/config.exe $2 $3 $4 $5
sleep 1
echo "done for $1"

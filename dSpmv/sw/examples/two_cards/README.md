# Summary

This example design demonstrate the steps of running SpMV operation on two Alveo U55C cards. One card is running the computation,
namely the dsmpm_compute.xclbin, and the other card is running the vector store, namely dspmv_store.xclbin.


# Steps for running dspmv on 2 u55c cards from two hosts 

# clone the repository

git clone https://github.com/Xilinx/HPC.git --recursive

navigate to dSpmv/sw/examples/two_cards

# partition the matrix given in the matrix list

1. source ./setup.sh

2. pushd ../../../../L2/sparse/tests/fp64/matrix_partition/

3. ./run_test.sh

4. python ../spmv/python/gen_vectors.py --gen_vec --mtx_list ./test_matrices.txt --vec_path ./vec_dat

5. popd

# reset u55c cards

../../../../xans/examples/network_config/run_reset.sh ./config.txt

# build executable

1. change the config.txt to reflect your setup of the alveo cards and host machines. The
field of the config.txt are explained below:

hostname ip_address xclbin_file_with_absolute_path u55c_card_id

you can run

xbutil scan to find the alveo u55c card id

2. run the following script to compile the host executables

./run_compile.sh ./config.txt

# set up IP addresses and UDP sockets of 2 Alveo cards on two host machines

./run_netConfig.sh ./config.txt ./ip.txt

# run dspmv on two hosts 

./run_dspmv.sh config.txt matrix_list.txt

or

## on second host 

./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/hostname2/dspmv.exe ./hostname2_0_sockets.txt ./ip.txt ../../../../L2/sparse/tests/fp64/matrix_partition/vec_dat nasa2910

## on first host 

./build_dir.hw.xilinx_u55c_gen3x16_xdma_base_2/hostname1/dspmv.exe ./hostname1_0_sockets.txt ./ip.txt ../../../../L2/sparse/tests/fp64/matrix_partition/sig_dat ../../../../L2/sparse/tests/fp64/matrix_partition/vec_dat nasa2910

# build .xclbin

1. set up Vitis 2021.1_released environments

2. navigate to dSpmv/hw/designs/dspmv/dspmv_compute

3. make cleanall PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u55c_gen3x16_xdma_2_202110_1 TARGET=hw INTERFACE=0

4. make xclbin PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u55c_gen3x16_xdma_2_202110_1 TARGET=hw INTERFACE=0

5. navigate to dSpmv/hw/designs/dspmv/dspmv_store

6. make cleanall PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u55c_gen3x16_xdma_2_202110_1 TARGET=hw INTERFACE=0

7. make xclbin PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u55c_gen3x16_xdma_2_202110_1 TARGET=hw INTERFACE=0

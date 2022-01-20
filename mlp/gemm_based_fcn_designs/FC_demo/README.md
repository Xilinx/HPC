# QRes MKL Benchmark

## MKL setup

download oneAPI Base Toolkit for Linux and install it, follow the steps in the installer see page: https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html?operatingsystem=linux&distributions=webdownload&options=online


for example

``
    wget https://registrationcenter-download.intel.com/akdlm/irc_nas/18236/l_BaseKit_p_2021.4.0.3422.sh

``

then source the setvars.sh under oneapi folder

## Usage

### Data

All inputs, weights, biases and output reference data has to be put in a
directory, for example "./data/". (please copy file NN356.bin, NN356inputs.bin, NN356outputs.bin to data)

### RUN

Source the oneapi setup for setting up MKLROOT variable

    > source <oneapi>setvars.sh

Make sure all necessary data are in a directory, then run:

    > make run DATA_PATH=./data/ BATCH_SIZE=2048 NUM_THREAD=32

By default, Makefile option DUMP_RESULT will be enabled. It will save the CPU results in data folder, so FCN application could use the data for compare as well. 

    > make run DATA_PATH=./data/ BATCH_SIZE=2048 NUM_THREAD=32
    > cd ../applications/FC_sigmoid/    
    > ./test_fcn.exe ./xclbin fcn.xclbin ../../FC_demo/data 20 2048 356 30 30 20 20 3

### Benchmark

Source `<oneapi install>oneapi/setvars.sh ` for proper oneapi setup before running following benchmarks.

Make sure all necessary data are in a directory, then run:

For Double Type, original benchmark code from Quantico

    1. change the SRCS in the Makefile to use QRes_mkl_bench2.cpp

    2. In QRes_mkl_bench2.cpp, uncomment line 17 "#define USE_DOUBLE_PRECISION" to use double precision

    3. source benchmark.sh


For Single precision Type, original benchmark code from Quantico

    1. change the SRCS in the Makefile to use QRes_mkl_bench2.cpp

    2. source benchmark.sh

For Single precision Type, original benchmark code from Quantico + removing caching effect

    1. change the SRCS in the Makefile to use QRes_mkl_bench3.cpp

    2. source benchmark.sh

For Double Type, benchmark code using same matrix operation order as FPGA 

    1. change the SRCS in the Makefile to use QRes_mkl_bench.cpp

    2. In QRes_mkl_bench.cpp, uncomment line 17 "#define USE_DOUBLE_PRECISION" to use double precision

    3. source benchmark.sh

For Single precision Type, benchmark code using same matrix operation order as FPGA

    1. change the SRCS in the Makefile to use QRes_mkl_bench.cpp

    2. source benchmark.sh

For Single precision Type, benchmark code using same matrix operation order as FPGA + removing caching effect

    1. change the SRCS in the Makefile to use QRes_mkl_bench4.cpp

    2. source benchmark.sh

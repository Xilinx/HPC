# QRes MKL Benchmark

## Environment setup

Follow the steps from the link below to install and setup MKL environment variables
https://xilinx.github.io/Vitis_Libraries/blas/2020.1/user_guide/L3/L3_benchmark_gemm.html#benchmarking-intel-math-kernel-library-mkl

## Usage

### Data

All inputs, weights, biases and output reference data has to be put in a
directory, for example "./data/".

### Make 

For silence runing mode:

``
    make
``

In order to write output results to ./data/ directory and make a comparison with
golden reference, please build with option:

``
    make DUMP_RESULT=1
``

### RUN

Make sure all necessary data are in a directory, then run:

``
    make run DATA_PATH=./data/ BATCH_SIZE=2048 NUM_THREADS=32
``

### Benchmark

For Single precision Type, original benchmark code from Quantico, make sure all necessary data are in a directory, then run:

For Single precision Type, original benchmark code from Quantico

    source benchmark.sh


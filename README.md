# Accelerating applications in FPGA-enabled HPC systems
This HPC repository provides solutions and POCs for accelerating applications in heterogeneous HPC systems, especially FPGA-enabled HPC system. The focus of this repository is to provide ready-to-use products that can benefit from the performance, power and cost improvement offered by FPGA-enabled HPC systems. Those system setups can be a single CPU server with multiple FPGA accelerator cards, a multiple of those servers or even a multiple HPC servers with different accelerator cards, e.g. GPU and FPGA cards.
The pure FPGA-related accelerator implementation can be found in L1 and L2 directories. Just like the file organization of Vitis_libraries, the L1 directory contains the basic hardware routines intented to be used in bigger hardware accelerator implementation. The L2 directory contains the standalone FPGA accelerator implementation. That is, those accelerators or CUs (compute units) can be addressed or used to interact with CPU code / software via XRT functions calls.
The application level solutions that can be deployed on multiple FPGA cards and multiple HPC servers can be found directly under the root directory of the repository. Currently, we provide two solutions, one is called xMLP, which supports high-precision MLP inference on FPGA-enable HPC servers. The other is called xPCG, which provide preconditioned conjugate gradient solver solution for the above mentioned HPC server as well. More detailed documentation is availabe at [Xilinx HPC Products Github Pages](https://xilinx.github.io/HPC/index.html) 
## License

Licensed using the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).

    Copyright 2021-2022 Xilinx, Inc.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
    Copyright 2021-2022 Xilinx, Inc.

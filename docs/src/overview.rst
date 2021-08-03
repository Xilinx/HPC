.. 
   Copyright 2019 - 2021 Xilinx, Inc.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
       http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

.. _brief:

***************************
Xilinx® Alveo™ HPC Products
***************************

PCG Alveo Product
=================

Engineering simulation software allows product engineers to predict the physical characteristics of
products under development with greater accuracy, and as a result, to deliver high
quality products with a much shorter development cycle. One of the key components
in those engineering simulation applications is a set of numerical solvers. The PCG (Preconditioned Conjugate Gradient) 
solver is a well-known method for solving large sparse linear systems.
The solver is widely used in the following simulation systems.

* Structural mechanics simulation
* Thermal analysis
* Dynamic fluid simulation

To be more precise, the PCG method is used to solve a large sparse linear system **Ax=b** for vector x,
where A is a symmetric positive definite matrix.

The Xilinx® PCG Alveo™ Product provides a hardware-accelerated PCG solver for use with a Xilinx Alveo U280 accelerator
card.  Particularly when the density of matrix A is less than 10% or each dimension of matrix A is 
more than a half million, the PCG Alveo Product can greatly boost the speed of your PCG computations relative to
alternative PCG solver implementations.
 
FPGA Accelerated JPCG Solver
----------------------------

The algorithm below describes a PCG solver with a Jacobi preconditioner, referred to as "JPCG" in PCG Alveo Product
documentation.

.. figure:: /images/JPCG_algo.png
   :alt: JPCG algorithm
   :scale: 100%
   :align: center
   
   Algorithm source: https://en.wikipedia.org/wiki/Conjugate_gradient_method#The_preconditioned_conjugate_gradient_method 

The Xilinx® PCG Alveo Product consists of a software component, supplied as a shared library (.so file), and
a hardware component, supplied as an Alveo card program file (XCLBIN file).  The shared library
links with your C application, and the XCLBIN file loads onto the Alveo accelerator card.

As shown in the diagram below, the shared library consists of a set of C API functions, labeled as "xJPCG APIs,"
and their underlying software implementation, labeled as "xJPCG SW Lib."  The XCLBIN file installs the hardware-based
PCG solver, labeled as "xJPCG ciruit," into the Alveo card.

Your C application calls the API functions, providing matrix A, vector b, and other parameters.  The software library
stores matrix A and vector b in CPU memory for reuse in future API calls before sending them on to the PCG solver
in the Alveo card. When the computation has completed, the library returns solution vector x from the Alveo card
to your application.

.. figure:: /images/PCG_stack.png
   :alt: PCG Alveo product diagram
   :scale: 100%
   :align: center

Run JPCG on the Alveo U280 Accelerator Card
-------------------------------------------

The `Xilinx® Alveo™ U280 Data Center accelerator cards <https://www.xilinx.com/products/boards-and-kits/alveo/u280.html>`_
provide optimized acceleration across a wide range of workloads. Designed for deployment in any server,
the Alveo U280 accelerator card has the following features:

* Built on Xilinx UltraScale+ architecture 
* Small power consumption footprint, maximum 225W 
* 100 Gbps networking I/O
* PCIe Gen4
* HBM  

The PCG Alveo Product includes an example application to demonstrate how to integrate the hardware-accelerated
PCG solver into your own application.

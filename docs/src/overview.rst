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

**************************
Xilinx Alveo HPC Products
**************************

PCG Solver
==========

Engineering simulation software allows companies to predict the behavior of
their future products more accurately, and as a result, to deliver high
quality products with a much shorter development cycle. One of the key components
in those engineering simulation applications is a set of numerical solvers. PCG (Preconditioned Conjugate Gradient) 
solver is a well know method for solving large sparse linear systems.
It is widely used in following simulation systems.

* Structural mechanics simulation
* Thermal analysis
* Dynamic fluid simulation

To be more precise, PCG method is used to solve a large sparse linear system **Ax=b**, where A is a symmetric 
positive definite matrix. When the density of matrix A is less than 10% or the dimension of the matrix A is 
more than half million, the PCG solver product here will help you to speed up your solution.
 
FPGA accelerated JPCG function
-------------------------------

The algorithm below describes a PCG solver with Jacobi preconditioner, referred to as JPCG in our document.

.. figure:: /images/JPCG_algo.png
   :alt: JPCG algorithm
   :scale: 100%
   :align: center
   
   Algorithm source: https://en.wikipedia.org/wiki/Conjugate_gradient_method#The_preconditioned_conjugate_gradient_method 

As shown below, the Xilinx PCG Alveo product currently provides a set of C functions 
called xJPCG APIs to offload the JPCG solver from users' C applicaiton to Xilinx Alveo U280 accelerator card.
The offloading process is essentially a data transfer process between the software and hardware 
components of Xilinx PCG Alveo product. The hardware component, also called xJPCG circuit, realizes JPCG solver
on the Alveo card. The software component also called xJPCG software library transfers the input data, namely
matrix A and right hand vector b to the xJPCG circuit, and read the solution, the x vector back from the xJPCG circuit.

.. figure:: /images/PCG_stack.png
   :alt: PCG Alveo product diagram
   :scale: 100%
   :align: center

Run JPCG on Alveo U280 Accelerator Card
----------------------------------------

The `Xilinx® Alveo™ U280 Data Center accelerator cards <https://www.xilinx.com/products/boards-and-kits/alveo/u280.html>`_
provide optimized acceleration across a wide range of workload. Alveo U280 accelerator card is 
designed for deployment in any server with the following features:

* Built on Xilinx UltraScale+ architecture 
* Small power consumption footprint, maximum 225W 
* 100 Gbps networking I/O
* PCIe Gen4
* HBM  

An example is included in this repository to show the general usage model of accelerating JPCG solver with the Xilinx PCG Alveo product. 

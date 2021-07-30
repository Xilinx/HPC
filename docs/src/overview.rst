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

==========================
Xilinx Alveo HPC Products
==========================

PCG Solver
----------

Engineering simulation software allows companies to predict the behavior of
their future products more accurately, and as a result, to deliver high
quality products with a much shorter development cycle. One of the key components
in those engineering simulation applications is a set of numerical solvers. PCG (Preconditioned Conjugate Gradient) 
solver is a well know method for solving large sparse linear systems.
It is widely used in following simulation systems.

* Structural mechanics simulation
* Thermal analysis
* Dynamic fluid simulation

To be more precise, PCG method is used to solve a large sparse linear system Ax=b, where A is a symmetric 
positive definite matrix. When the density of matrix A is less than 10% or the dimension of the matrix A 
is at least half million, the PCG solver product here will speed up your solution. The algorithm below 
describes a PCG solver with Jacobi preconditioner.

.. figure:: /images/JPCG_algo.png
   :alt: JPCG algorithm
   :scale: 100%
   :align: center
   
   Algorithm source: https://en.wikipedia.org/wiki/Conjugate_gradient_method#The_preconditioned_conjugate_gradient_method 

FPGA accelerated JPCG function
********************************
As shown below, in the installation package of the Xilinx PCG product,
a set of C functions and an FPGA configuration file are provided. After
the installation, users can direcly call the C functions to offload the
JPCG (Jacobi Preconditioned Conjugate Gradient) solver to Alveo FPGA card.



.. figure:: /images/PCG_stack.png
   :alt: PCG stack 
   :scale: 100%
   :align: center

The usage of the C functions is illustrated below. An example is also
provided to demostrate more usage details.

* create_JPCG_handle: create device handle for JPCG accelerator. This function is only called once, normally at the beginning of the application.

* JPCG_coo: launch JPCG accelerate with COO matrix formation storage and different options to allow re-using the matrix partitioning.

* destroy_JPCG_HANDLE: release device handle for JPCG accelerator. This function is only called once, normally at the end of the application.

The hardware architecture of the JPCG accelerator is shown in the following image.
The main architecture highlights are:

* supporting float64 data type

* running at 240MHz

* scalable kernel-to-kernel streaming structure

* software configurable loop control circuit

* dedicated HBM channels for storing matrix and vectors to avoid data movement overhead


.. figure:: /images/JPCG_arch.png
   :alt: JPCG hardware architecture 
   :scale: 100%
   :align: center

Run JPCG on Alveo U280 Card
******************************

The `Xilinx® Alveo™ U280 Data Center accelerator cards <https://www.xilinx.com/products/boards-and-kits/alveo/u280.html>`_
provides optimized accelerator across a wide range of workload. Alveo U280 is 
designed for deployment in any server with the following features:

* Built on Xilinx UltraScale+ architecture 
* Small power consumption footprint, maximum 225W 
* 100 Gbps networking I/O
* PCIe Gen4
* HBM  

An example is included in this repository to show the general development flow of accelerating JPCG solver on Alveo U280 card. 

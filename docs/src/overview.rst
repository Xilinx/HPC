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

General use cases
-----------------

Engineering simulation softwares allow companies to predict the behavior of
their future products more accurately, and as a result, to deliver high
quality products with a much short development cycle. One of the key components
in those engineering simulation applications is a set of numerical solvers. PCG solver
is a well know method for solving large sparse symmetric positive definite linear systems.
It is widely used in following simulation systems.

* Structural mechanics simulation
* Thermal analysis
* Dynamic fluid simulation

Integrate Xilinx PCG solver into LS-DYNA thermal analysis product
------------------------------------------------------------------
Xilinx PCG solver is integrated into the usermats of LS-DYNA thermal analysis product via 
a C function interface. The low level hardware detail are transparent to LS-DYNA users. The
C function is provided in this repository and can be called directly from any other libraires
of the usermat. 

PCG Solver
----------
PCG solver is widely used to solve a large sparse linear system Ax=b, where A is a symmetric 
positive definite matrix. The algorithm below describes a PCG solver with Jacobi preconditioner.
The major part of the algorithm is an iterative loop which contains a SPMV (for sparse systems) or 
GEMV operation (for dense systems) and a groupt of vector operations. When the dimension of the
matrix is significant smaller than the matrix entries (none zero entries for sparse matrix), the 
time spent on each iteration is dominated by the matrix vector multiplication. 

.. figure:: /images/JPCG_algo.png
   :alt: JPCG algorithm
   :scale: 100%
   :align: center
   
   Algorithm source: https://en.wikipedia.org/wiki/Conjugate_gradient_method#The_preconditioned_conjugate_gradient_method 



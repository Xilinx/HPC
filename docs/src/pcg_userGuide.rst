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

.. _module_guide:

*************************
PCG Product User Guide
*************************

The PCG Product allows you to use a Xilinx Alveo accelerator card to solve a system with huge number of linear equations,
which are normally represented as Ax = b. Here A is a sparse coefficient matrix, which has multi-million dimensions
and multi-million none-zero entries; b is a dense vector; and x is the dense vector returned by the PCG product.

Using C APIs
==================

Follow the steps below to use the APIs.

#. Instantiate a xJPCG handle
#. Call xJPCG solver functions to solve a system **Ax = b** 
#. Destroy the xJPCG handle at the end of the application

Instantiate a xJPCG handle
---------------------------
To instantiate a xJPCG handle, please give the device ID of the Alveo U280 accelerator card installed on your server,
and the location of the FPGA program file (XCLBIN file name). You can use command **xbutil scan** to find out the device
ID of the Alveo U280 card. More details of **xbutil** can be found `here <https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/xbutilutility.html>`_.

.. code-block:: bash

   #include "pcg.h" 
   
   xJPCG_Handle_t* pHandle = NULL;
   int err = xJPCG_createHandle(&pHandle, deviceId, xclbinPath);


Call the xJPCG solver functions to solve a system **Ax = b**
-----------------------------------------------------------------
Two xJPCG solver functions are provided in the product to cater for different matrix storage formats.
If the matrix is stored in **COO** format, please use function xJPCG_cooSolver.
If the matrix is stored in **CSC** format and only the top or bottom triangular part of the matrix is stored,
please use function xJPCG_cscSym. The detailed information about **COO** and **CSC**
sparse matrix storage formats can be found `here <https://en.wikipedia.org/wiki/Sparse_matrix>`_.

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n, nnz, rowIdx, colIdx, data, matJ, b, x, maxIter, tol,
                                        &iter, &res,  XJPCG_MODE_DEFAULT);

Destroy the xJPCG handle
------------------------
At the end of the application, please call xJPCG_destroyHandl function to release all resources used by the product.

   
Solving multiple systems
========================
In an application, sometimes multiple systems need to be solved one after another. These systems
might have following features
#. The coefficient matrix between the current system being solved is completely different from the previous one.
That is, both the indices and NNZ values in the coefficient matrix are different.
#. The coefficient matrix of the current system has same structure, meaning same indices with the previous system that has been solved.
#. The coefficient matrix of the current system is completely same as the previous one.

Completely different coefficient between two systems
---------------------------------------------------- 

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, n2, nnz2, rowIdx2, colIdx2, data2, matJ2, b2, x2, maxIter, tol,
                                        &iter2, &res2,  XJPCG_MODE_DEFAULT);

Coefficient matrice of two systems have same structure
------------------------------------------------------- 

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data2, matJ2, b2, x2, maxIter, tol,
                                        &iter2, &res2,  XJPCG_MODE_KEEP_NZ_LAYOUT);

Coefficient matrice of two systems are completely same
------------------------------------------------------- 

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b2, x2, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_KEEP_MATRIX);

Error handling
==============

Linking your application
========================


xJPCG API reference 
===================

.. toctree::
      :maxdepth: 2 

   pcg_api.rst

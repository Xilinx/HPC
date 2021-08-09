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

****************************
PCG Alveo Product User Guide
****************************

The PCG Alveo Product allows you to use a Xilinx Alveo accelerator card to solve a large system of linear equations,
represented by the equation **Ax = b**. In this equation **A** is a sparse coefficient matrix with
potentially millions of rows and columns and millions of non-zero entries, **b** is a dense vector,
and **x** is the dense vector to solve for.

Using the C APIs
================

This section provides a brief overview of the sequence of API function calls your application needs to make.
For more details about function arguments and behavior, please refer to the API Reference.

Follow the steps below to use the APIs.

#. Instantiate an xJPCG handle
#. Call xJPCG solver functions to solve a system **Ax = b** 
#. Destroy the xJPCG handle

Instantiate an xJPCG handle
---------------------------
To instantiate a xJPCG handle, please call the ``xJPCG_createHandle()`` function, supplying the device ID of
the Alveo U280 accelerator card installed on your server and the location of the FPGA program file (XCLBIN file).
You can use the ``xbutil scan`` command to determine the device ID of the Alveo U280 card. Details about
``xbutil`` can be found `here <https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/xbutilutility.html>`_.

.. code-block:: bash

   #include "pcg.h" 
   
   xJPCG_Handle_t* pHandle = NULL;
   int err = xJPCG_createHandle(&pHandle, deviceId, xclbinPath);

Creating a handle opens a "session" with the Alveo card. The handle itself is a pointer to the implementation details
for the session.  You can copy the ``xJPCG_Handle_t *`` pointer and use both the original and copy to refer to the same
session, but you should not call ``xJPCG_createHandle()`` twice for the same Alveo card, as the PCG Alveo Product supports
only one session per Alveo card.

Call the xJPCG solver functions to solve a system **Ax = b**
------------------------------------------------------------
The PCG Alveo Product provides two xJPCG solver functions to cater for different matrix storage formats.
If the matrix is stored in **COO** format, please use the ``xJPCG_cooSolver()`` function.
If the matrix is stored in **CSC** format and only the top or bottom triangular part of the matrix is stored,
please use the ``xJPCG_cscSymSolver()`` function. Detailed information about **COO** and **CSC**
sparse matrix storage formats can be found `here <https://en.wikipedia.org/wiki/Sparse_matrix>`_.

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n, nnz, rowIdx, colIdx, data, matJ, b, x, maxIter, tol,
                                        &iter, &res,  XJPCG_MODE_DEFAULT);

Destroy the xJPCG handle
------------------------
When you have finished solving systems, please call the ``xJPCG_destroyHandle()`` function to release all resources used
by the product.

   
Solving Multiple Systems
========================
In your application you may need to solve multiple systems in sequence. To maximize performance by reusing
coefficient matrix data, the solver functions support three reuse scenarios, listed below.  In these scenarios,
"previous system" refers to the data passed to the most recent solver function call, while "current system"
refers to a new solver function call.  For data reuse to succeed, you must make both function calls in the same session
(that is, with the same xJPCG handle).

#. **Completely new coefficient matrix**: The coefficient matrix of the current system is completely different
   from that of the previous system. That is, both the indices and NNZ (non-zero) values in the coefficient matrix
   are different.

#. **Coefficient matrix structure preserved**: The coefficient matrix of the current system has the same structure
   (that is, the same indices) as that of the previous system, but the NNZ values are different.

#. **Coefficient matrix completely preserved**: The coefficient matrix of the current system is identical to that
   of the previous one.

The following subsections show you how to use the solver functions for multiple runs in each reuse scenario.
In each of these scenarios the solver function is called twice: once for the first call of a session and a second
time to demonstrate the reuse.  Input arguments in **bold** indicate values that have changed from the first call to the
second.  The remaining input arguments have values which are identical between the two calls.

Note that you must pass the coefficient matrix to the second solver function call even if the matrix is unchanged.
Also, the **b** vector is free to change with every solver function call.

Completely new coefficient matrix
---------------------------------

When completely replacing the coefficient matrix in the second call, use the ``XJPCG_MODE_DEFAULT`` option, as
shown below.

.. raw:: html

   <pre>
   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, <b>n2</b>, <b>nnz2</b>, <b>rowIdx2</b>, <b>colIdx2</b>, <b>data2</b>, <b>matJ2</b>, <b>b2</b>, x2, maxIter, tol,
                                        &iter2, &res2,  XJPCG_MODE_DEFAULT);
   </pre>

Coefficient matrix structure preserved
--------------------------------------

When replacing the coefficient matrix's values while keeping the matrix's structure (indices) in the second call,
use the ``XJPCG_MODE_KEEP_NZ_LAYOUT`` option, as shown below.

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data2, matJ2, b2, x2, maxIter, tol,
                                        &iter2, &res2,  XJPCG_MODE_KEEP_NZ_LAYOUT);

Coefficient matrix completely preserved
---------------------------------------

When retaining the entire cofficient matrix for the second call, including both structure (indices) and non-zero
values, use the ``XJPCG_MODE_KEEP_MATRIX`` option, as shown below.

.. code-block:: bash

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b1, x1, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_DEFAULT);

   int err = xJPCG_cooSolver(pHandle, n1, nnz1, rowIdx1, colIdx1, data1, matJ1, b2, x2, maxIter, tol,
                                        &iter1, &res1,  XJPCG_MODE_KEEP_MATRIX);

Error Handling
==============

Most of the API functions return a status code of enumeration type ``XJPCG_Status_t``.  Successful function calls
return enumerator ``XJPCG_STATUS_SUCCESS``.  All other returned enumerators indicate an error, for which you can
retrieve a detailed error message with the ``xJPCG_getLastMessage()`` function.  You can also retrieve the status code
of the most recent failure with the ``xJPCG_peekAtLastStatus()`` function.

Both error retrieval functions take a handle pointer to fetch the most recent error for that handle.  For most types
of errors, the handle pointer must be valid, except for dynamic loading errors (see the next section, `Linking Your
Application`), which disregard the handle pointer, as those errors are global.  Note that the ``xJPCG_createHandle()``
function returns a valid handle to use with error retrieval functions even if initialization failed.

Linking Your Application
========================

You have a few choices for how to link the API code into your application:

- Linking directly with the PCG shared library (.so)
- Linking with the PCG dynamic loader archive (.a)
- Including the PCG dynamic loader source file (.c)

Linking directly (.so)
----------------------

The simplest method of linking the API into your application is to link directly with the shared library (.so),
placing a run-time dependency of your application on the shared library.  Simply add the following arguments to
your link line:

::

    -L/opt/xilinx/apps/hpc/pcg/<version>/lib -lXilinxPcg


where ``<version>`` is the version of the PCG Alveo Product.

Linking with the dynamic loader archive (.a)
--------------------------------------------

To avoid having a run-time dependency on the shared library, but instead load the shared library on demand
(internally using ``dlopen()``), you can link with the loader archive by adding the following arguments to
your link line:

::

    -L/opt/xilinx/apps/hpc/pcg/<version>/lib -lXilinxPcg_loader -ldl

where ``<version>`` is the version of the PCG Alveo Product.

Including the dynamic loader source file (.c)
---------------------------------------------

Another way to avoid a run-time dependency on the shared library is by including the loader source file in
a header or source file of your program:

::

    #define XILINX_PCG_INLINE_IMPL
    #include "pcg.h"

    // Code that uses the API goes here

    #include "pcg_loader.c"


Note that you will still have to include ``-ldl`` on your link line to pull in the standard dynamic loading library.

The loader source file is located in ``/opt/xilinx/apps/hpc/pcg/<version>/src``, where ``<version>`` is the version
of the PCG Alveo Product.  Note the macro definition that comes before the inclusion of ``pcg.h``.

*TIP:** When using either dynamic loading technique, if the order of symbol loading causes unexplained behavior in
your application, you can try adding ``libXilinxPcgStatic.so`` to the list of pre-loaded shared libraries,
as explained in `this article <https://stackoverflow.com/questions/426230/what-is-the-ld-preload-trick>`_.



xJPCG API reference 
===================

.. toctree::
      :maxdepth: 2 

   rst_pcg/pcg_api.rst

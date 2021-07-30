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

The PCG Product allows you to use a Xilinx Alveo accelerator card to solve huge number of linear equations,
which are normally represented as Ax = b. Here A is a sparse coefficient matrix, which has multi-million dimensions
and multi-million none-zero entries; b is a dense vector; and x is the dense vector returned by the PCG product.

Using C APIs
==================

Alveo accelerator card storage capacity
========================================

Error handling
==============

Linking your application
========================


.. toctree::
      :maxdepth: 2 

   pcg_api.rst

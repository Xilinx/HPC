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

=========================
Vitis HPC Product Library
=========================

This repository provides FPGA-based heterogeneous solutions for accelerating HPC applications.
It currently includes following product:

* PCG (Preconditioned Conjugate Gradient) solver 

.. toctree::
   :caption:  Overview
   :maxdepth: 1

   overview.rst
 
.. toctree::
   :caption: PCG Solver Usage Examples
   :maxdepth: 1

    pcg_examples.rst

.. toctree::
   :caption: PCG APIs 
   :maxdepth: 2 

   user_guide/L1/L1.rst
   user_guide/L2/L2.rst
   user_guide/L3/L3.rst

.. toctree::
  :caption: PCG Benchmark Results  
  :maxdepth: 1 

  benchmark.rst

.. toctree::
   :caption:  Release Notes
   :maxdepth: 1

   release.rst

Requirements
------------

Software Platform
~~~~~~~~~~~~~~~~~

For hardware users, who want to generate FPGA configuration file (.xclbin file), Vitits and XRT 2021.1 have to be installed.

For pure software or product users, we provide an installation package that include all files to allow you to run PCG standalone product.
This installation package has been tested and verified on the following OS.

- RHEL/CentOS 7.4

PCIE Accelerator Card
~~~~~~~~~~~~~~~~~~~~~

PCG products are designed to work with Alveo U280 cards.

License
-------

Licensed using the `Apache 2.0 license <https://www.apache.org/licenses/LICENSE-2.0>`_.

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

Trademark Notice
----------------

    Xilinx, the Xilinx logo, Artix, ISE, Kintex, Spartan, Virtex, Zynq, and
    other designated brands included herein are trademarks of Xilinx in the
    United States and other countries.  All other trademarks are the property
    of their respective owners.

Index
-----

* :ref:`genindex` 

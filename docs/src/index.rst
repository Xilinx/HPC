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
It currently includes following products:

* PCG (Preconditioned Conjugate Gradient) solver and the thermal analysis product integrated with LS-DYNA

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

   pyenvguide.rst
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

This library is designed to work with Vitis 2021.1 and later, and therefore inherits the system requirements of Vitis and XRT.

Supported operating systems are RHEL/CentOS 7.4, 7.5 and Ubuntu 16.04.4 LTS, 18.04.1 LTS.
With CentOS/RHEL 7.4 and 7.5, C++11/C++14 should be enabled via
`devtoolset-6 <https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/>`_.

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

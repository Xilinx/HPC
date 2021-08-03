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

===========================
Install Xilinx PCG Product
===========================

* Power off the server, plug Xilinx Alveo U280 card in the PCIe x 16 slot on the motherboard. Power on the server.

* Get the installation package `xilinx-pcg-1.0_7.4.1708-x86_64.rpm from 
  Xilinx website <https://www.xilinx.com/member/forms/download/design-license-xef.html?filename=xilinx-pcg-1.0_7.4.1708-x86_64.rpm>`_ 

* Instll Xilinx PCG Alveo Product and dependencies (XRT AND U280 platform packages)

.. code-block:: bash

  sudo yum install ./xilinx-pcg-1.0_7.4.1708-x86_64.rpm (if you don't have PCG product on your machine)
  or 
  sudo yum reinstall ./xilinx-pcg-1.0_7.4.1708-x86_64.rpm (if you have PCG product on your machine)


* Flash the Alveo U280 card if it is not already running with the shell
  xilinx_u280_xdma_201920_3. Cold reboot the machine after flashing is done.

==================================
Collaborate on Xilinx PCG Product
==================================

Following the instructions below if you want to collaborate and contribute to
Xilinx PCG Alveo product.

* Clone HPC repository

.. code-block:: bash

   git clone https://github.com/Xilinx/hpc.git

* Build hardward (.xclbin file).

.. code-block:: bash

   cd pcg/hw
   make

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

* Run the example test.

.. code-block:: bash

   cd pcg/sw
   make
   cd pcg/sw/examples/c
   make
   make run

* Build installation package

.. code-block:: bash

   cd pcg/sw
   make dist

The installation package (.rpm file) will be stored under pcg/sw/package.

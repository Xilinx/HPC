===========================
Install Xilinx PCG Product
===========================

* Power off the server, plug Xilinx Alveo U280 card in the PCIe x 16 slot on the motherboard. Power on the server.

* Get the installation package `xilinx-pcg-install-1.0.tar.gz from 
  Xilinx website <https://www.xilinx.com/member/forms/download/design-license-xef.html?filename=xilinx-pcg-install-1.0.tar.gz>`_ 

* Instll Xilinx PCG Alveo Product and dependencies (XRT AND U280 platform packages)

.. code-block:: bash

   tar xzf xilinx-pcg-install-1.0.tar.gz
   cd xilinx-pcg-install && ./install.sh

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

* Build and install PCG package. CentOS yum package manager is used as an example.

.. code-block:: bash

   cd pcg
   make dist
   sudo yum install --reinstall ./package/xilinx-pcg-1.0_7.4-x86_64.rpm

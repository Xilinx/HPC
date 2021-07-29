# Build xclbin for PCG

## 1. Environment setting

Set env for XILINX_VIVADO, XILINX_VITIS and XILINX_XRT are required to build xclbin

## 2. Build xclbin

Please be noticed that this process will take couple of hours to finish.
TARGET could be either hw or hw_emu.

    make run TARGET=hw PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u280_xdma_201920_3




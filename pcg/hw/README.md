# Build xclbin for PCG

## 1. Set environment variables

Set environment variables XILINX_VIVADO, XILINX_VITIS and XILINX_XRT via following commands (assume you are using bash on Linux systems).

    source /opt/xilinx/Vitis/2021.1/settings64.sh
    source /opt/xilinx/xrt/setup.sh

## 2. Build xclbin

Please note that this process will take a couple of hours to finish.

    make build TARGET=hw PLATFORM_REPO_PATHS=/opt/xilinx/platforms DEVICE=xilinx_u280_xdma_201920_3




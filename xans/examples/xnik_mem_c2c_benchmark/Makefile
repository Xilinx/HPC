#
# Copyright 2019-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.4
#

# ####################################### Help Section #####################################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<x86>"
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make run TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<x86>"
	$(ECHO) "      Command to run application in emulation or on board."
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform> HOST_ARCH=<x86>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) ""
	$(ECHO) "  make host HOST_ARCH=<x86>"
	$(ECHO) "      Command to build host application."
	$(ECHO) ""

# ##################### Setting up default value of TARGET ##########################
TARGET ?= hw_emu

# ################### Setting up default value of DEVICE ##############################
DEVICE ?= xilinx_u55c_gen3x16_xdma_base_2

# ###################### Setting up default value of HOST_ARCH ####################### 
HOST_ARCH ?= x86

# #################### Checking if DEVICE in blacklist #############################

# ######################## Setting up Project Variables #################################
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/xans/*}')
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XFLIB_DIR = $(XF_PROJ_ROOT)

# ######################### Include environment variables in utils.mk ####################
include ./utils.mk
XDEVICE := $(call device2xsa, $(DEVICE))
TEMP_DIR := _x_temp.$(TARGET).$(XDEVICE)
TEMP_REPORT_DIR := $(CUR_DIR)/reports/_x.$(TARGET).$(XDEVICE)
BUILD_DIR := build_dir.$(TARGET).$(XDEVICE)/$(HOST)
BUILD_REPORT_DIR := $(CUR_DIR)/reports/_build.$(TARGET).$(XDEVICE)
EMCONFIG_DIR := $(BUILD_DIR)

# ######################### Setting up Host Variables #########################
#Include Required Host Source Files
HOST_SRCS += $(XFLIB_DIR)/L2/common/src/sw/xNativeFPGA.cpp
HOST_SRCS += $(XFLIB_DIR)/xans/sw/src/cmac.cpp
HOST_SRCS += $(XFLIB_DIR)/xans/sw/src/networklayer.cpp
HOST_SRCS += $(XFLIB_DIR)/xans/examples/xnik_mem_c2c_benchmark/src/main.cpp

CXXFLAGS += -DXANS_numInf=1 -DXANS_mtuBytes=1472 -DXANS_memBits=256 -DXANS_maxInstrs=64 -DXANS_netDataBits=512 -DXANS_userBits=1 -DXANS_destBits=16 -DXANS_maxPackets=2048 
CXXFLAGS += -I$(XFLIB_DIR)/L1/blas/include/hw
CXXFLAGS += -I$(XFLIB_DIR)/L1/hpc/include
CXXFLAGS += -I$(XFLIB_DIR)/L2/common/include
CXXFLAGS += -I$(XFLIB_DIR)/xans/hw/xnik/include
CXXFLAGS += -I$(XFLIB_DIR)/xans/sw/include

ifeq ($(TARGET),sw_emu)
CXXFLAGS += -D SW_EMU_TEST
endif

ifeq ($(TARGET),hw_emu)
CXXFLAGS += -D HW_EMU_TEST
endif

# ######################### Host compiler global settings ############################
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -std=c++1y -Wall -Wno-unknown-pragmas -Wno-unused-label
CXXFLAGS += -fmessage-length=0 -g -O0
CXXFLAGS += -I$(CUR_DIR)/src/ 
LDFLAGS += -lxrt_core -L$(XILINX_XRT)/lib -lpthread -lrt -lstdc++ -luuid -lxrt_coreutil -Wno-unused-label -Wno-narrowing -DVERBOSE

# ################### Setting package and image directory #######################
XCLBIN_PATH := /home/lingl/backup/vnx_basic_if3.xclbin 

deviceID = 0

EXE_NAME := c2c_benchmark.exe
EXE_FILE := $(BUILD_DIR)/$(EXE_NAME)
HOST_ARGS := $(XCLBIN_PATH) ${deviceID} 

# ######################### Setting Targets of Makefile ################################

.PHONY: all clean cleanall docs emconfig 
all:  check_xclbin $(EXE_FILE) emconfig 

.PHONY: host
host: check_xrt check_sysroot $(EXE_FILE)

check_xclbin:
ifeq (,$(wildcard $(USER_BUILT_XCLBIN)))
ifeq (,$(wildcard $(PRE_BUILT_XCLBIN)))
	$(error Please build or checkout xclbin)
endif
endif

# ################# Setting Rules for Host (Building Host Executable) ################
$(EXE_FILE): $(HOST_SRCS) | check_xrt
	mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(XPLATFORM) --od $(EMCONFIG_DIR)

# ###############Setting Essential Checks And Running Rules For Vitis Flow #############
run: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) $(EXE_FILE) $(HOST_ARGS)
else
	$(EXE_FILE) $(HOST_ARGS)
endif

# ################################# Cleaning Rules ##################################
cleanh:
	-$(RMDIR) $(EXE_FILE) vitis_* TempConfig system_estimate.xtxt *.rpt .run/
	-$(RMDIR) src/*.ll _xocc_* .Xil dltmp* xmltmp* *.log *.jou *.wcfg *.wdb sample_link.ini sample_compile.ini obj* bin* *.csv *.jpg *.jpeg *.png

cleank:
	-$(RMDIR) $(BUILD_DIR)/*.xclbin _vimage *xclbin.run_summary qemu-memory-_* emulation/ _vimage/ pl* start_simulation.sh *.xclbin
	-$(RMDIR) _x_temp.*/_x.* _x_temp.*/.Xil _x_temp.*/profile_summary.* xo_* _x*
	-$(RMDIR) _x_temp.*/dltmp* _x_temp.*/kernel_info.dat _x_temp.*/*.log 
	-$(RMDIR) _x_temp.* 

cleanall: cleanh cleank
	-$(RMDIR) $(BUILD_DIR)  build_dir.* emconfig.json *.html $(TEMP_DIR) $(CUR_DIR)/reports *.csv *.run_summary $(CUR_DIR)/*.raw package_* run_script.sh .ipcache *.str
	-$(RMDIR) $(XFLIB_DIR)/common/data/*.xe2xd* $(XFLIB_DIR)/common/data/*.orig*

	-$(RMDIR) $(AIE_CONTAINERS) $(CUR_DIR)/Work $(CUR_DIR)/*.xpe $(CUR_DIR)/hw.o $(CUR_DIR)/*.xsa $(CUR_DIR)/xnwOut aiesimulator_output .AIE_SIM_CMD_LINE_OPTIONS

clean: cleanh

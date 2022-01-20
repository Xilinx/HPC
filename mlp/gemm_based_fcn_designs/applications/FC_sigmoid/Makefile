 
#
# Copyright 2019 Xilinx, Inc.
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
#

# -----------------------------------------------------------------------------
#                          project common settings

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))

.SECONDEXPANSION:

# -----------------------------------------------------------------------------
#                            common tool setup


.PHONY: help

help::
	@echo ""
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make host "
	@echo "      Command to generate host."
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated non-hardware files."
	@echo ""
	@echo "  make cleanall"
	@echo "      Command to remove all the generated files."
	@echo ""

ifeq (,$(XILINX_XRT))
XILINX_XRT = /opt/xilinx/xrt
endif
export XILINX_XRT
.PHONY: check_xrt
check_xrt:
ifeq (,$(wildcard $(XILINX_XRT)/lib/libxilinxopencl.so))
	@echo "Cannot locate XRT installation. Please set XILINX_XRT variable." && false
endif

export PATH := $(XILINX_XRT)/bin:$(PATH)

ifeq (,$(LD_LIBRARY_PATH))
LD_LIBRARY_PATH := $(XILINX_XRT)/lib
else
LD_LIBRARY_PATH := $(XILINX_XRT)/lib:$(LD_LIBRARY_PATH)
endif

# -----------------------------------------------------------------------------
# BEGIN_XF_MK_USER_SECTION
# -----------------------------------------------------------------------------

MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/mlp/*}')
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XFLIB_DIR = $(XF_PROJ_ROOT)

# -----------------------------------------------------------------------------

SRC_DIR = $(CUR_DIR)

EXE_NAME = test_fcn
HOST_ARGS =

SRCS = test_fcn_as_function.cpp
EXAMPLE = fcn_example.cpp

CXXFLAGS += -g -I$(XILINX_XRT)/include -I$(XFLIB_DIR)/mlp/gemm_based_fcn_designs/sw/include/ 

# -----------------------------------------------------------------------------
# END_XF_MK_USER_SECTION
# -----------------------------------------------------------------------------

.PHONY: all
all: host 

OBJ_DIR_BASE ?= obj
BIN_DIR_BASE ?= bin


OBJ_DIR = $(CUR_DIR)/$(OBJ_DIR_BASE)$(BIN_DIR_SUFFIX)
BIN_DIR = $(CUR_DIR)/$(BIN_DIR_BASE)$(BIN_DIR_SUFFIX)

RUN_ENV =
OBJ_FILES = 
EXTRA_OBJS = 

CXX := g++
CC := gcc

#CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
CXXFLAGS += -O0 -std=c++1y -fPIC -Wextra -Wall -Wno-ignored-attributes -Wno-unused-parameter -Wno-unused-variable
CFLAGS +=
LDFLAGS += -pthread -L$(XILINX_XRT)/lib
LDFLAGS += -L$(XILINX_XRT)/lib  -lz -lstdc++ -lrt -pthread -lxrt_core -ldl -luuid -lxrt_coreutil

EXE_EXT ?= exe
EXE_FILE ?= $(EXE_NAME)$(if $(EXE_EXT),.,)$(EXE_EXT)

OUT_HW_DIR = out_hw
CONFIG :=${OUT_HW_DIR}/config_info.dat

TARGET = hw

ifneq ("$(wildcard $(CONFIG))","")
	include ${CONFIG}
endif

$(EXE_FILE): $(SRCS) | check_xrt 
	$(CXX) -D BLAS_numKernels=$(BLAS_numKernels) -D BLAS_gemmMBlocks=$(BLAS_gemmMBlocks) -D BLAS_gemmKBlocks=$(BLAS_gemmKBlocks) -D BLAS_gemmNBlocks=$(BLAS_gemmNBlocks) -D BLAS_ddrWidth=$(BLAS_ddrWidth) -D BLAS_numKernels=$(BLAS_numKernels) -D BLAS_runFcn=1 -o $@ $^  $(CXXFLAGS) $(LDFLAGS)
	
example: $(EXAMPLE) | check_xrt 
	$(CXX) -o fcn_example.exe $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY: host
host: check_xrt $(EXE_FILE)

xclbin: download_xclbin

build: download_xclbin

download_xclbin:
ifeq (,$(wildcard ./xf_blas_xclbins))
	wget -nc "https://www.xilinx.com/bin/public/openDownload?filename=vitis_BLAS_library_r1.0_xclbin.tar"
	tar -zxvf openDownload?filename=vitis_BLAS_library_r1.0_xclbin.tar
endif

# -----------------------------------------------------------------------------
#                                clean up

clean:
ifneq (,$(OBJ_DIR_BASE))
	rm -rf $(CUR_DIR)/$(OBJ_DIR_BASE)*
endif
ifneq (,$(BIN_DIR_BASE))
	rm -rf $(CUR_DIR)/$(BIN_DIR_BASE)*
endif
	rm -rf *.exe

cleanall: clean 
	rm -rf *.log plist $(DATA_STAMP)

.PHONY: run 

run: host

check: run




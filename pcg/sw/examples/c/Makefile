#
# Copyright 2020-2021 Xilinx, Inc.
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

# Which Alveo device to use for running tests.  Run with "make run DEVICE_ID=<n>" to use device <n>
DEVICE_ID ?= 1

# Run make using "make DEBUG=1" to build debuggable executables

# Location of cosine similarity Alveo product 
ifndef XILINX_PCG
    XILINX_PCG = /opt/xilinx/apps/HPC/pcg
    export XILINX_PCG
endif

# Location of XRT and XRM (for "run" target only)
ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

#the default PROD_VERSION as the latest version
ifndef PROD_VERSION 
    PROD_VERSION = 1.0
endif

# Location of Alveo product if installed from sources
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/examples/c/Makefile}')

LIB_PATH = $(XF_PROJ_ROOT)/staging/lib
INCLUDE_PATH = $(XF_PROJ_ROOT)/staging/include
XCLBIN_PATH = $(XF_PROJ_ROOT)/staging/xclbin

ifeq ($(wildcard $(LIB_PATH)/*.so),)
    LIB_PATH = $(XILINX_PCG)/${PROD_VERSION}/lib
    INCLUDE_PATH = $(XILINX_PCG)/${PROD_VERSION}/include
    XCLBIN_PATH = $(XILINX_PCG)/${PROD_VERSION}/xclbin
endif


#CFLAGS = -fPIC -pthread -I$(INCLUDE_PATH) -I$(XILINX_XRT)/include \
#    -Wall -Wextra -Wno-unknown-pragmas 
CFLAGS = -fPIC -pthread -I$(INCLUDE_PATH) -Wall -Wextra

ifdef DEBUG
CFLAGS += -O0 -g
else
CFLAGS += -O3
endif


LDFLAGS = -L$(LIB_PATH) -lXilinxPcg -lpthread -lstdc++

$(info PROJ_ROOT="$(XF_PROJ_ROOT)")
$(info LIB_PATH="$(LIB_PATH)")
$(info INCLUDE_PATH="$(INCLUDE_PATH)")



all: example

example: example.c
	gcc $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: run clean

run:
	@set -e; \
	. $(XILINX_XRT)/setup.sh; \
	export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH; \
	./example $(DEVICE_ID) $(XCLBIN_PATH)/*.xclbin

clean:
	rm -f example

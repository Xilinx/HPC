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



pyGenMat=genMat.py
SRCS=../../../src/sw/c++/gen_signature.cpp ../../../../../utils/src/sw/binFiles.cpp ./main.cpp
TARGET=gen_signature.exe

sig_path=./sig_dat/
mtx_list=test_matrices.txt
SPARSE_maxRows=4096
SPARSE_maxCols=4096
SPARSE_hbmChannels=16
SPARSE_parEntries=4
SPARSE_accLatency=8
SPARSE_hbmMemBits=256
MTX_NAME=nasa2910

MACROS +=-D SPARSE_maxRows=$(SPARSE_maxRows) \
	 -D SPARSE_maxCols=$(SPARSE_maxCols) \
	 -D SPARSE_hbmChannels=$(SPARSE_hbmChannels) \
	 -D SPARSE_parEntries=$(SPARSE_parEntries) \
	 -D SPARSE_accLatency=$(SPARSE_accLatency) \
	 -D SPARSE_hbmMemBits=$(SPARSE_hbmMemBits)

HOST_ARGS = ${sig_path}/${MTX_NAME}/

CXX	= $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
ifeq ($(LD_LIBRARY_PATH),)
export LD_LIBRARY_PATH := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/lib64
else
export LD_LIBRARY_PATH := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/lib64:$(LD_LIBRARY_PATH)
endif
CFLAGS	= -g -O0 -std=c++14 -I../../../include/sw/fp64 -I../../../../../utils/include/sw
CFLAGS += ${MACROS}


${TARGET}: ${SRCS}
	$(CXX) ${CFLAGS} $^ -o $@
	
build: ${TARGET}

data_gen: 
	@mkdir -p ${sig_path} 
	python3 ${pyGenMat} --mtx_list ${mtx_list} --sig_path ${sig_path}

run: ${TARGET} data_gen 
	@mkdir -p ${sig_path} 
	./${TARGET} ${HOST_ARGS} 

clean:
	@rm -rf ${TARGET}

cleanall:
	@rm -rf ${TARGET}
	@rm -rf ${sig_path}


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
SRCS=srcs/gen_signature.cpp
TARGET=gen_signature.exe

sig_path=./sig_dat/
mtx_list=test_matrices.txt
MAX_ROWS=4096
MAX_COLS=4096
CHANNELS=16
PAR_ENTRIES=4
ACC_LATENCY=8
MEM_BITS=256
MTX_NAME=nasa2910

MACROS +=-D MAX_ROWS=$(MAX_ROWS) \
	 -D MAX_COLS=$(MAX_COLS) \
	 -D CHANNELS=$(CHANNELS) \
	 -D PAR_ENTRIES=$(PAR_ENTRIES) \
	 -D ACC_LATENCY=$(ACC_LATENCY) \
	 -D MEM_BITS=$(MEM_BITS)

HOST_ARGS = ${sig_path}/${MTX_NAME}/

CXX	= g++
CFLAGS	= -O2 -std=c++14 -I ./includes/
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


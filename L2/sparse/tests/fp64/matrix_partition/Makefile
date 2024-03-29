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
pyGenSig=../spmv/python/gen_signature.py
pyGenVec=../spmv/python/gen_vectors.py
SRCS=../../../src/sw/c++/gen_signature.cpp ./main.cpp
TARGET=./gen_signature.exe

sig_path=./sig_dat/
mtx_list=test_matrices.txt
SPARSE_dataType=double
SPARSE_maxRows=4096
SPARSE_maxCols=4096
SPARSE_hbmChannels=16
SPARSE_parEntries=4
SPARSE_accLatency=8
SPARSE_hbmMemBits=256
MTX_NAME=bcsstm39

MACROS +=-D SPARSE_dataType=$(SPARSE_dataType) \
	 -D SPARSE_maxRows=$(SPARSE_maxRows) \
	 -D SPARSE_maxCols=$(SPARSE_maxCols) \
	 -D SPARSE_hbmChannels=$(SPARSE_hbmChannels) \
	 -D SPARSE_parEntries=$(SPARSE_parEntries) \
	 -D SPARSE_accLatency=$(SPARSE_accLatency) \
	 -D SPARSE_hbmMemBits=$(SPARSE_hbmMemBits)

HOST_ARGS = ${sig_path}/${MTX_NAME}/ 1 0

CXX	= g++
CFLAGS	= -g -O0 -std=c++11 -I../../../include/sw/fp64 -I../../../../../utils/include/sw -pthread 
CFLAGS += ${MACROS} 


${TARGET}: ${SRCS}
	$(CXX) ${CFLAGS} $^ -o $@
	
build: ${TARGET}

data_gen: 
	@mkdir -p ${sig_path} 
	python3 ${pyGenMat} --mtx_list ${mtx_list} --sig_path ${sig_path}
	python3 ${pyGenVec} --gen_vec --mtx_list ${mtx_list} --vec_path ./vec_dat

run: ${TARGET} 
	@mkdir -p ${sig_path} 
	${TARGET} ${HOST_ARGS} 

verify: run
	python ${pyGenSig} --check --mtx_list test_matrices.txt --sig_path sig_dat/

clean:
	@rm -rf ${TARGET}

cleanall:
	@rm -rf ${TARGET}
	@rm -rf ${sig_path}


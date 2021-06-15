#
# Copyright 2019-2020 Xilinx, Inc.
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

pyGenDat=${XFLIB_DIR}/pcg/tests/pcg_spmv_c++/host/genMat.py
mtxList = ${XFLIB_DIR}/pcg/tests/pcg_spmv_c++/host/test.txt

dataPath = ./$(BUILD_DIR)/data/

deviceID = 0
maxIter = 5000
tol=1e-12
mtxName = ted_B

HOST_ARGS += ${maxIter} ${tol} ${dataPath} $(mtxName) ${deviceID}

data_gen:
	rm -rf ${dataPath}
	python ${pyGenDat}  --mtx_list ${mtxList} --sig_path ${dataPath}

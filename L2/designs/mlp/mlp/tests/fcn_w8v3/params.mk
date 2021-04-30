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

pyGenMat=${XFLIB_DIR}/L2/mlp/src/sw/python/genFcn.py
p_n = 8
p_m = 16
p_b = 201
HPC_deviceId = 0
HPC_dataType = float
HPC_activation = sigmoid

dataDir = ./$(BUILD_DIR)/data_${p_b}_${p_m}_${p_n}/
HOST_ARGS += $(p_b) $(p_m) $(p_n) $(dataDir) ${HPC_deviceId}

data_gen:
	mkdir -p ${dataDir} 
	python3 ${pyGenMat} --p_b $(p_b) --p_m $(p_m) --p_n $(p_n) --path ${dataDir} --datatype ${HPC_dataType} --act ${HPC_activation}

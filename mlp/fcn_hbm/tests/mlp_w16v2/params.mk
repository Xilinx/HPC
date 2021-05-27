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

pyGenMat=${XFLIB_DIR}/L2/designs/mlp/fcn_hbm/src/sw/python/genMLP.py
batch = 10000
num = 3
layers=784 64 64 16
HPC_configDevice = $(CUR_DIR)/devices.json
HPC_dataType = float
HPC_activation = sigmoid

dataDir = ./$(BUILD_DIR)/data_${batch}_${num}/
HOST_ARGS += $(batch) ${num} ${layers} $(dataDir) ${HPC_configDevice}


data_gen: $(dataDir)/sig

$(dataDir)/sig:
	mkdir -p ${dataDir} 
	python3 ${pyGenMat} --batch $(batch) --layers ${layers} --path ${dataDir} --datatype ${HPC_dataType} --act ${HPC_activation}
	touch $(dataDir)/sig

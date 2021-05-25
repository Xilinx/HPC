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

keras=${XFLIB_DIR}/L2/designs/mlp/fcn_hbm/src/sw/python/keras_example.py

LIB_DIR = ${CUR_DIR}/lib
LIB_FILE = ${LIB_DIR}/${EXE_NAME}

pythonApi: ${LIB_FILE}

${LIB_FILE}: ${EXE_FILE}
	@mkdir -p ${LIB_DIR}
	@mv ${EXE_FILE} ${LIB_FILE}
	@rm -rf ${BUILD_DIR}

MODEL_NAME=mnist
MODEL_PATH=./models

train: 
	python ${keras} --train --model_path ${MODEL_PATH} --model_name ${MODEL_NAME} 

inf:
	python ${keras} --kinf --model_path ${MODEL_PATH} --model_name ${MODEL_NAME} 

export PYTHONPATH:=${LIB_DIR}:$$PYTHONPATH
xinf:
	python ${keras} --xinf --model_path ${MODEL_PATH} --model_name ${MODEL_NAME} --device_config ${CUR_DIR}/devices.json

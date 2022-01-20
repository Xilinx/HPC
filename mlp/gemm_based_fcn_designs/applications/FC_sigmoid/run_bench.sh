#!/bin/bash

# Copyright 2021-2022 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

XCLBIN_PATH=$1
XCLBIN_FILE=$2
MODEL_NUMBER=$3

make host OUT_HW_DIR=$1

batch_list="204800"
k0=356
n0=30
k1=30
n1=20
k2=20
n2=3
logs=()

for batch in $batch_list; do
 python data_gen.py --model $3 --batch $batch --sizes $k0 $n0 $k1 $n1 $k2 $n2
 ./test_fcn.exe $1 data_$batch $3 $batch $k0 $n0 $k1 $n1 $k2 $n2 | tee log-$batch.txt
 logs="$logs log-$batch.txt"
done


egrep -h ^DATA_CSV $logs |  head -1  > perf_QRes_20NN_Single.csv
egrep -h ^DATA_CSV $logs |  grep -v Traces >> perf_QRes_20NN_Single.csv

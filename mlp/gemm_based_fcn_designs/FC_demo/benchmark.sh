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


make benchmark NUM_THREADS=96 | tee log
egrep -h ^DATA_CSV log |  head -1  > perf_QRes_20NN_Single.csv
egrep -h ^DATA_CSV log |  grep -v Type >> perf_QRes_20NN_Single.csv

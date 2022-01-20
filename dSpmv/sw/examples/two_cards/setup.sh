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

(return 0 2>/dev/null) && sourced=1 || sourced=0

if [ $sourced -eq 0 ]; then
  echo "This script should be sourced"
  exit 1
fi

tools_kit_path=$(readlink -f $(dirname ${BASH_SOURCE[0]}))

if [ ! -d $tools_kit_path/py3env ]; then
  (cd ${tools_kit_path}; ./create_py3env.sh)
fi

unset PYTHONPATH
source ${tools_kit_path}/py3env/bin/activate

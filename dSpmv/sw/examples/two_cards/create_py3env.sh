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

unset PYTHONPATH
PATH=$PATH:/usr/bin

if [ ! -f py3env/bin/activate ]; then
  echo "INFO: creating python3 venv..."
  python3 -m venv py3env
fi

if [ -f py3env/bin/activate ]; then
  source py3env/bin/activate
  if [ ! -f py3env/bin/sphinx-build ]; then
    echo "INFO: pip installing in venv..."
    pip install -U pip
    pip install wheel
    pip install -r requirements.txt
  fi
else
  echo "ERROR: venv activate script does not exist. Skipping pip install"
fi

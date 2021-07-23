#!/bin/bash

# Copyright 2021 Xilinx, Inc.
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

set -e

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -f test-list     : File containing list of test matrices.  Defaults to ./test.txt"
    echo "  -o output-dir    : Directory to which to write test data.  Defaults to ./data"
    echo "  -h               : Print this help message"
}

dataFile=./test.txt
dataOutPath=./data

while getopts ":f:o:h" opt
do
case $opt in
    f) dataFile=$OPTARG;;
    o) dataOutPath=$OPTARG;;
    h) usage; exit 0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

mkdir -p $dataOutPath
venvPath=$dataOutPath/venv

# Make virtual environment for running the genMat Python script if it doesn't already exist

if [ -d $venvPath ]; then
    . $venvPath/bin/activate
else
    echo "Setting up Python environment..."
    python3 -m venv $venvPath
    . $venvPath/bin/activate
    pip install numpy
    pip install scipy
fi

python3 $SCRIPTPATH/genMat.py --mtx_list $dataFile --sig_path $dataOutPath

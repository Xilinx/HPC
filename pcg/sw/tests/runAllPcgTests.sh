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

set -e

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -f test-list   : File containing list of test matrices.  Defaults to ./test.txt"
    echo "  -o output-dir  : Directory to which to write test data.  Defaults to ./data"
    echo "  -r results-dir : Directory to which to write test results.  Defaults to ./results"
    echo "  -d device-id   : The device number of the Alveo card to use.  Defaults to 1"
    echo "  -h             : Print this help message"
}

# Set up XRT

xrtPath=$XILINX_XRT
if [ "$xrtPath" == "" ]; then
    $xrtPath=/opt/xilinx/xrt
fi
if [ ! -d $xrtPath ]; then
    echo "ERROR: XRT not found.  Please install XRT or set $$XILINX_XRT to the location of XRT."
    exit 1
fi

. $xrtPath/setup.sh

# Process command line options

dataFile=./test.txt
dataOutPath=./data
resultsPath=./results
deviceId=1

while getopts ":f:o:r:d:h" opt
do
case $opt in
    f) dataFile=$OPTARG;;
    o) dataOutPath=$OPTARG;;
    r) resultsPath=$OPTARG;;
    d) deviceId=$OPTARG;;
    h) usage; exit 0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

# Download and prepare the data

$SCRIPTPATH/../utils/genMat.sh -f $dataFile -o $dataOutPath

# From run_list.sh: read each test from the test list file and run pcgtest on each test

rm -rf $resultsPath
mkdir -p $resultsPath
binPath=$SCRIPTPATH/../Release
export LD_LIBRARY_PATH=$binPath:$LD_LIBRARY_PATH

file="$dataFile"
while read -r line; do
  if [ "$line" == "" ]; then
    continue
  fi
  name="$line"
  filename=$(basename -- $name)
  mtxname=${filename%%.*}
  echo "=======Running $mtxname=========="
  logname=$resultsPath/$mtxname.log
  $binPath/pcgtest $SCRIPTPATH/../staging/xclbin/pcg_xilinx_u280_xdma_201920_3.xclbin \
        5000 1e-12 $dataOutPath $mtxname 1 $deviceId |& tee $logname
done < "$file"

egrep -h ^DATA_CSV $resultsPath/*.log | grep matrix_name | head -1 > $resultsPath/perf_pcg_fpga.csv
egrep -h ^DATA_CSV $resultsPath/*.log | grep -v matrix_name >> $resultsPath/perf_pcg_fpga.csv
echo "Results are in $resultsPath/perf_pcg_fpga.csv"

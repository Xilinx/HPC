#!/bin/env bash

if [[ $# -eq 0 ]]; then
    echo "no file list provided"
    exit 0
fi

logs=()
n=0
file="$1"
while read -r line; do
  name="$line"
  filename=$(basename -- $name)
  mtxname=${filename%%.*}
  echo "=======Running $mtxname=========="
  ./build_dir.hw.xilinx_u280_xdma_201920_3/host.exe ./xclbin/cgSolver_ddrParParam_hbmRbParam.xclbin 5000 1e-12 build_dir.hw.xilinx_u280_xdma_201920_3/data $mtxname 1 1 |& tee log$n.txt
  logs="$logs log$n.txt"
  n=$((n+1))
done < "$file"

egrep -h ^DATA_CSV $logs | grep matrix_name | head -1 > perf_pcg_fpga.csv
egrep -h ^DATA_CSV $logs | grep -v matrix_name >> perf_pcg_fpga.csv
echo Results are in perf_pcg_fpga.csv

rm -rf $logs


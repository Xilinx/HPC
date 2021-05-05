make benchmark NUM_THREADS=96 | tee log
egrep -h ^DATA_CSV log |  head -1  > perf_QRes_20NN_Single.csv
egrep -h ^DATA_CSV log |  grep -v Type >> perf_QRes_20NN_Single.csv

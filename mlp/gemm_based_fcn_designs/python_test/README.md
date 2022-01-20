# usage

1. cd mlp/gemm_based_fcn_designs/sw/src/python_api
2. make api
3. export PYTHONPATH=../sw/src/python_api/:../applications/FC_sigmoid/
4. python test_fcn.py  --xclbin ../applications/FC_sigmoid/xclbin/fcn.xclbin --cfg ../applications/FC_sigmoid/xclbin/config_info.dat --lib ../sw/src/python_api/lib/xfblas.so

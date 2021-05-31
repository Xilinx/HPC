rm -rf sig_dat/
rm -rf vec_dat/
python ./python/gen_signature.py --partition --mtx_list ./test_matrices.txt --sig_path ./sig_dat
python ./python/gen_vectors.py --gen_vec --mtx_list ./test_matrices.txt --vec_path ./vec_dat
make cleanall
make run PLATFORM_REPO_PATHS=/opt/xilinx/platforms TARGET=hw
exit


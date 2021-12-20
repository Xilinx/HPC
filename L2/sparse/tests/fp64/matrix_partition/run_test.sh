make cleanall
make data_gen
make
shopt -s dotglob
find sig_dat/* -prune -type d | while IFS= read -r d; do 
    ./gen_signature.exe $d 1 0
done

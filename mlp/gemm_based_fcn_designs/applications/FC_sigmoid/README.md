### Instructions for Running application on U250    
#### Copy the generated fcn.xclbin to xclbin directory
#### Check your LD_LIBRARY_PATH, it should be set as follows:
     LD_LIBRARY_PATH=/opt/xilinx/xrt/lib 
#### Compile the Host code. 
     make clean OUT_HW_DIR=./xclbin
     make host OUT_HW_DIR=./xclbin
#### Run following command to generate data for Quantico's benchmark models
    (Usage) 20 model, with batch size=204800, model size 356 30 30 20 20 3   
    > python data_gen.py --model 20 --batch 204800 --sizes 356 30 30 20 20 3
    (Usage) 20 model, with batch size=3741, model size 43 13 13 10 10 4
    > python data_gen.py --model 20 --batch 3741 --sizes 43 13 13 10 10 4
            
#### Run following command to run the inference with the generated models and data
    (Usage)./test_fcn.exe <xclbin directory> <path_to_data> <num_of_model> <xclbin File Name> BATCH_SIZE K0 N0 K1 N1 K2 N2
    For Example,     
    > ./test_fcn.exe ./xclbin fcn.xclbin data_204800 20 204800 356 30 30 20 20 3
    > ./test_fcn.exe ./xclbin fcn.xclbin data_3741 20 3741 43 13 13 10 10 4
    Note: you can use following command  to automatically run step 4 with multiple batch sizes. (model size and batch size need to be manually modified in run_bench.sh)




### Instructions for Running application on AWS
#### Download aws-fpga repository and source the Required Vitis 2021.1 setup
    > git clone https://github.com/aws/aws-fpga.git $AWS_FPGA_REPO_DIR  
    > source $AWS_FPGA_REPO_DIR/vitis_setup.sh ;
    > source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh
#### Copy the generated fcn.awsxclbin to xclbin directory
#### Compile the Host code. 
     make clean OUT_HW_DIR=./xclbin
     make host OUT_HW_DIR=./xclbin
#### Run following command to generate data for Quantico's benchmark models
    (Usage) 20 model, with batch size=204800, model size 356 30 30 20 20 3   
    > python data_gen.py --model 20 --batch 204800 --sizes 356 30 30 20 20 3
    (Usage) 20 model, with batch size=3741, model size 43 13 13 10 10 4
    > python data_gen.py --model 20 --batch 3741 --sizes 43 13 13 10 10 4
            
#### Run following command to run the inference with the generated models and data
    (Usage)./test_fcn.exe <xclbin directory> <path_to_data> <num_of_model> <xclbin File Name> BATCH_SIZE K0 N0 K1 N1 K2 N2
    For Example,     
    > ./test_fcn.exe ./xclbin fcn.awsxclbin data_204800 20 204800 356 30 30 20 20 3
    > ./test_fcn.exe ./xclbin fcn.awsxclbin data_3741 20 3741 43 13 13 10 10 4
    Note: you can use following command  to automatically run step 4 with multiple batch sizes. (model size and batch size need to be manually modified in run_bench.sh)
    > ./run_bench.sh ./xclbin fcn.awsxclbin 20

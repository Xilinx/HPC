# Instructions for Building application (fcn.awsxclbin)
### 1. (For AWS Setup) Download aws-fpga repository and source the Required Vitis 2021.1 setup
    git clone https://github.com/aws/aws-fpga.git $AWS_FPGA_REPO_DIR  
    > source $AWS_FPGA_REPO_DIR/vitis_setup.sh ;
    > source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh
### 2. Set top level environment variable 
    > export PROJ_QUANTICO=<top level directory>
### 3. Build AFI for AWS fcn.awsxclbin
    > cd $PROJ_QUANTICO/fcn_aws_1cu; 
    > export PLATFORM_REPO_PATHS=<Path where platform is installed>
      e.g export PLATFORM_REPO_PATHS=/proj/xbuilds/2020.2_daily_latest/internal_platforms
    > make build TARGET=hw

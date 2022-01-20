# Instructions for Building application (fcn.xclbin)
### Set top level environment variable 
    export PROJ_QUANTICO=<top level directory>
### Build fcn.xclbin
    > cd $PROJ_QUANTICO/fcn_3cu; 
    > export PLATFORM_REPO_PATHS=<Path where platform is installed>
      e.g export PLATFORM_REPO_PATHS=/proj/xbuilds/2020.2_daily_latest/internal_platforms
    > make build TARGET=hw

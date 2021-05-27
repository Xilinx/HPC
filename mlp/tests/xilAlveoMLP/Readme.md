# Xilinx Alveo MLP

## Usage

### Dependencies

#### conda mlp environment
Run following command to activate mlp environment from conda.
```
conda activate mlp
```

#### MKL library
Install MKL library and run the following command to setup the environment.
```
source mklvars.sh intel64
```

### Build Shared Library
```
make host TARGET=hw 
```

### Configuration

Please make sure the information in the configuration file [**devices.json**](./devices.json) is correct. 

### Steps

#### Train Model
```
make train
```

#### Run Inference on CPU
```
make inf
```


#### Run Inference on FPGA
```
make xinf
```

# Xilinx Alveo MLP

## Usage

### Dependencies

Please check the information at page [MLP Design on HBM Devices](../../Readme.md) for dependencies. 

### Build Shared Library
```
make pythonAPi
```

### Configuration

Please make sure the information in the configuration file **devices.json** is correct. 

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

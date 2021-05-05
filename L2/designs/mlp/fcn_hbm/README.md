# MLP Design on HBM Devices

## Usage

### Dependency

#### Python

Use following command to create mlp environment with  propery python libraries

```
conda env create -f environment.yml

```

#### Library

[**Vitis_Libraries**](https://gitenterprise.xilinx.com/FaaSApps/Vitis_Libraries)

#### File Structure

```
root
  ├── HPC
  |   └── L2
  |       └── designs
  |
  └── Vitis_Libraries
      ├── blas
      └── hpc

```

### Tests


#### Emulation

```
make run TARGET=sw_emu/hw_emu p_in=32 p_out=32 p_batch=200
```

#### Build Hardware

```
make build TARGET=hw
```

#### Run on FPGA

```
make run TARGET=hw p_in=32 p_out=32 p_batch=200
```

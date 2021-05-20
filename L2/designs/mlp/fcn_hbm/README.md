# MLP Design on HBM Devices

## Usage

### Dependency

#### Python

Use the following command to create mlp environment with proper python libraries

```
conda env create -f ../environment.yml

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

Here is a brief usage for software and haredware emulation, and how to build hardware and run it on FPGAs. 

#### Emulation

```
make run TARGET=sw_emu/hw_emu
```

#### Build Hardware

```
make build TARGET=hw
```

#### Run on FPGA

```
make run TARGET=hw
```

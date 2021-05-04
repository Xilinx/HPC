# MLP Design on HBM Devices

## Usage

### Dependency

[**Vitis_Libraries**](https://gitenterprise.xilinx.com/FaaSApps/Vitis_Libraries)

#### File Structure
.
+-- HPC
|   +-- L2
|       +-- designs
+-- Vitis_Libraries
|   +-- blas
|   +-- hpc

### Tests


#### Emulation

```
make run TARGET=sw_emu/hw_emu p_in=32 p_out=32 p_batch=200
```

#### Build Hardware

```
make build TARGET=hw
```

#### Run on Hardware

```
make run TARGET=hw p_in=32 p_out=32 p_batch=200
```

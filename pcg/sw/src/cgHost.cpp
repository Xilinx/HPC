/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/*
 * Copyright 2019-2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "impl/cgHost.hpp"

namespace xilinx_apps {
namespace pcg {
CGKernelControl::CGKernelControl(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelControl::setMem(void* p_instr, unsigned int p_instrBytes) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err;
    // Running the kernel
    m_buffer_instr = createDeviceBuffer(CL_MEM_READ_WRITE, p_instr, p_instrBytes, &l_each_err);
    l_err = l_err && l_each_err;
    // Setting Kernel Arguments
    err = m_kernel.setArg(0, m_buffer_instr);
    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_instr);
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;
    if (l_err == true && err == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

bool CGKernelControl::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    bool l_err = true;
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_instr);
    l_err = getBuffer(l_buffers);
    return l_err;
}

CGKernelStoreApk::CGKernelStoreApk(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelStoreApk::setMem(void* p_pk, unsigned int p_pkSize, void* p_Apk, unsigned int p_ApkSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err0, err1;
    // Running the kernel
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize, &l_each_err);
    l_err = l_err && l_each_err;
    // Setting Kernel Arguments
    err0 = m_kernel.setArg(1, m_buffer_pk);
    err1 = m_kernel.setArg(2, m_buffer_Apk);

    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_pk);
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err0 == CL_SUCCESS && err1 == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

CGKernelUpdatePk::CGKernelUpdatePk(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelUpdatePk::setMem(void* p_pk, unsigned int p_pkSize, void* p_zk, unsigned int p_zkSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err0, err1, err2;
    // Running the kernel
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_zk = createDeviceBuffer(CL_MEM_READ_WRITE, p_zk, p_zkSize, &l_each_err);
    l_err = l_err && l_each_err;

    // Setting Kernel Arguments
    int n_arg = 0;
    err0 = m_kernel.setArg(n_arg++, m_buffer_pk);
    err1 = m_kernel.setArg(n_arg++, m_buffer_pk);
    err2 = m_kernel.setArg(n_arg++, m_buffer_zk);

    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_zk);
    l_buffers.push_back(m_buffer_pk);
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err0 == CL_SUCCESS && err1 == CL_SUCCESS && err2 == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
bool CGKernelUpdatePk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    bool l_err = true;
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_pk);
    l_err = getBuffer(l_buffers);
    return l_err;
}

CGKernelUpdateRkJacobi::CGKernelUpdateRkJacobi(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelUpdateRkJacobi::setMem(void* p_rk,
                                    unsigned int p_rkSize,
                                    void* p_zk,
                                    unsigned int p_zkSize,
                                    void* p_jacobi,
                                    unsigned int p_jacobiSize,
                                    void* p_Apk,
                                    unsigned int p_ApkSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err0, err1, err2, err3, err4;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_rk = createDeviceBuffer(CL_MEM_READ_WRITE, p_rk, p_rkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_zk = createDeviceBuffer(CL_MEM_READ_WRITE, p_zk, p_zkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_jacobi = createDeviceBuffer(CL_MEM_READ_ONLY, p_jacobi, p_jacobiSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize, &l_each_err);
    l_err = l_err && l_each_err;

    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_zk);
    l_buffers.push_back(m_buffer_Apk);
    l_buffers.push_back(m_buffer_jacobi);

    // Setting Kernel Arguments
    int l_index = 0;
    err0 = m_kernel.setArg(l_index++, m_buffer_rk);
    err1 = m_kernel.setArg(l_index++, m_buffer_rk);
    err2 = m_kernel.setArg(l_index++, m_buffer_zk);
    err3 = m_kernel.setArg(l_index++, m_buffer_jacobi);
    err4 = m_kernel.setArg(l_index++, m_buffer_Apk);

    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;
    if (l_err == true && err0 == CL_SUCCESS && err1 == CL_SUCCESS && err2 == CL_SUCCESS && err3 == CL_SUCCESS &&
        err4 == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
bool CGKernelUpdateRkJacobi::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    bool l_err = true;
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_zk);
    l_err = getBuffer(l_buffers);
    return l_err;
}

CGKernelUpdateRk::CGKernelUpdateRk(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelUpdateRk::setMem(void* p_rk, unsigned int p_rkSize, void* p_Apk, unsigned int p_ApkSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err0, err1, err2;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_rk = createDeviceBuffer(CL_MEM_READ_WRITE, p_rk, p_rkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize, &l_each_err);
    l_err = l_err && l_each_err;

    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_Apk);

    // Setting Kernel Arguments
    int l_index = 0;
    err0 = m_kernel.setArg(l_index++, m_buffer_rk);
    err1 = m_kernel.setArg(l_index++, m_buffer_rk);
    err2 = m_kernel.setArg(l_index++, m_buffer_Apk);

    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;
    if (l_err == true && err0 == CL_SUCCESS && err1 == CL_SUCCESS && err2 == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
bool CGKernelUpdateRk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    bool l_err = true;
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_rk);
    l_err = getBuffer(l_buffers);
    return l_err;
}

CGKernelUpdateXk::CGKernelUpdateXk(FPGA* p_fpga) : Kernel(p_fpga) {}
bool CGKernelUpdateXk::setMem(void* p_xk, unsigned int p_xkSize, void* p_pk, unsigned int p_pkSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err0, err1, err2;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_xk = createDeviceBuffer(CL_MEM_READ_WRITE, p_xk, p_xkSize, &l_each_err);
    l_err = l_err && l_each_err;
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize, &l_each_err);
    l_err = l_err && l_each_err;

    l_buffers.push_back(m_buffer_xk);
    l_buffers.push_back(m_buffer_pk);

    // Setting Kernel Arguments
    int l_index = 0;
    err0 = m_kernel.setArg(l_index++, m_buffer_xk);
    err1 = m_kernel.setArg(l_index++, m_buffer_xk);
    err2 = m_kernel.setArg(l_index++, m_buffer_pk);

    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err0 == CL_SUCCESS && err1 == CL_SUCCESS && err2 == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
bool CGKernelUpdateXk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    bool l_err = true;
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_xk);
    l_err = getBuffer(l_buffers);
    return l_err;
}

KernelLoadNnz::KernelLoadNnz(FPGA* p_fpga) : Kernel(p_fpga) {}
bool KernelLoadNnz::setMem(std::vector<void*>& p_sigBuf, std::vector<unsigned int>& p_sigBufBytes) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err = CL_SUCCESS;
    unsigned int l_numChannels = p_sigBuf.size();
    m_buffers.resize(l_numChannels);
    std::vector<cl::Memory> l_buffers;
    for (unsigned int i = 0; i < l_numChannels; ++i) {
        m_buffers[i] = createDeviceBuffer(CL_MEM_READ_ONLY, p_sigBuf[i], p_sigBufBytes[i], &l_each_err);
        l_err = l_err && l_each_err;
        l_buffers.push_back(m_buffers[i]);
        err = m_kernel.setArg(i, m_buffers[i]);
    }
    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
KernelLoadCol::KernelLoadCol(FPGA* p_fpga) : Kernel(p_fpga) {}
bool KernelLoadCol::setParParamMem(void* p_paramBuf, unsigned int p_paramBufSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    m_buffers[0] = createDeviceBuffer(CL_MEM_READ_ONLY, p_paramBuf, p_paramBufSize, &l_each_err);
    l_err = l_err && l_each_err;
    err = m_kernel.setArg(0, m_buffers[0]);
    l_buffers.push_back(m_buffers[0]);
    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
bool KernelLoadCol::setXMem(void* p_xBuf, unsigned int p_xBufSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    m_buffers[1] = createDeviceBuffer(CL_MEM_READ_ONLY, p_xBuf, p_xBufSize, &l_each_err);
    l_err = l_err && l_each_err;
    err = m_kernel.setArg(1, m_buffers[1]);
    l_buffers.push_back(m_buffers[1]);
    // Copy input data to device global memory
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

KernelLoadRbParam::KernelLoadRbParam(FPGA* p_fpga) : Kernel(p_fpga) {}
bool KernelLoadRbParam::setMem(void* p_buf, unsigned int p_bufSize) {
    bool l_err = true;
    bool l_each_err = true;
    cl_int err;
    m_buffer = createDeviceBuffer(CL_MEM_READ_ONLY, p_buf, p_bufSize, &l_each_err);
    l_err = l_err && l_each_err;
    err = m_kernel.setArg(0, m_buffer);
    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer);
    l_each_err = sendBuffer(l_buffers);
    l_err = l_err && l_each_err;

    if (l_err == true && err == CL_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

xCgHost::xCgHost(std::string p_xclbinName) {
    init(p_xclbinName);
};
bool xCgHost::init(std::string p_xclbinName) {
    bool l_err = true;
    bool l_each_err = true;
    m_card.init(p_xclbinName, &l_each_err);
    l_err = l_err && l_each_err;
    m_krnCtl.fpga(&m_card);
    m_krnLoadArbParam.fpga(&m_card);
    m_krnLoadAval.fpga(&m_card);
    m_krnLoadPkApar.fpga(&m_card);
    m_krnStoreApk.fpga(&m_card);
    m_krnUpdatePk.fpga(&m_card);
    m_krnUpdateRkJacobi.fpga(&m_card);
    m_krnUpdateXk.fpga(&m_card);
    l_each_err = m_krnCtl.getCU("krnl_control");
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadAval.getCU("krnl_loadAval:{krnl_loadAval}");
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadPkApar.getCU("krnl_loadPkApar:{krnl_loadPkApar}");
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadArbParam.getCU("krnl_loadArbParam:{krnl_loadArbParam}");
    l_err = l_err && l_each_err;
    l_each_err = m_krnStoreApk.getCU("krnl_storeApk:{krnl_storeApk}");
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdatePk.getCU("krnl_update_pk");
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateRkJacobi.getCU("krnl_update_rk_jacobi");
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateXk.getCU("krnl_update_xk");
    l_err = l_err && l_each_err;
    if (l_err == true) {
        return true;
    } else {
        return false;
    }
}
bool xCgHost::sendMatDat(std::vector<void*>& p_nnzVal,
                         std::vector<unsigned int>& p_nnzValSize,
                         void* p_rbParam,
                         unsigned int p_rbParamSize,
                         void* p_parParam,
                         unsigned int p_parParamSize) {
    bool l_err = true;
    bool l_each_err = true;
    l_each_err = m_krnLoadAval.setMem(p_nnzVal, p_nnzValSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadArbParam.setMem(p_rbParam, p_rbParamSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadPkApar.setParParamMem(p_parParam, p_parParamSize);
    l_err = l_err && l_each_err;

    if (l_err == true) {
        return true;
    } else {
        return false;
    }
}
bool xCgHost::sendVecDat(void* p_pk,
                         unsigned int p_pkSize,
                         void* p_Apk,
                         unsigned int p_ApkSize,
                         void* p_zk,
                         unsigned int p_zkSize,
                         void* p_rk,
                         unsigned int p_rkSize,
                         void* p_jacobi,
                         unsigned int p_jacobiSize,
                         void* p_xk,
                         unsigned int p_xkSize) {
    bool l_err = true;
    bool l_each_err = true;
    l_each_err = m_krnLoadPkApar.setXMem(p_pk, p_pkSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnStoreApk.setMem(p_pk, p_pkSize, p_Apk, p_ApkSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdatePk.setMem(p_pk, p_pkSize, p_zk, p_zkSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateRkJacobi.setMem(p_rk, p_rkSize, p_zk, p_zkSize, p_jacobi, p_jacobiSize, p_Apk, p_ApkSize);
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateXk.setMem(p_xk, p_xkSize, p_pk, p_pkSize);
    l_err = l_err && l_each_err;

    if (l_err == true) {
        return true;
    } else {
        return false;
    }
}
bool xCgHost::sendInstr(void* p_instr, unsigned int p_instrSize) {
    bool l_err = m_krnCtl.setMem(p_instr, p_instrSize);
    return l_err;
}
bool xCgHost::run() {
    bool l_err = true;
    bool l_each_err = true;
    l_each_err = m_krnCtl.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadArbParam.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadAval.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnLoadPkApar.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnStoreApk.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdatePk.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateXk.enqueueTask();
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateRkJacobi.enqueueTask();
    l_err = l_err && l_each_err;
    if (l_err == true) {
        return true;
    } else {
        return false;
    }
}
bool xCgHost::getDat() {
    bool l_err = true;
    bool l_each_err = true;
    l_each_err = m_krnCtl.getMem();
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateXk.getMem();
    l_err = l_err && l_each_err;
    l_each_err = m_krnUpdateRkJacobi.getMem();
    l_err = l_err && l_each_err;
    if (l_err == true) {
        return true;
    } else {
        return false;
    }
}
void xCgHost::finish() {
    m_krnCtl.finish();
    m_krnLoadArbParam.finish();
    m_krnLoadAval.finish();
    m_krnLoadPkApar.finish();
    m_krnStoreApk.finish();
    m_krnUpdatePk.finish();
    m_krnUpdateXk.finish();
    m_krnUpdateRkJacobi.finish();
}
}
}
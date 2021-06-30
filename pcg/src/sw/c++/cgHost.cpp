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

#include "cgHost.hpp"

CGKernelControl::CGKernelControl(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelControl::setMem(void* p_instr, unsigned int p_instrBytes) {
    cl_int err;
    // Running the kernel
    m_buffer_instr = createDeviceBuffer(CL_MEM_READ_WRITE, p_instr, p_instrBytes);
    // Setting Kernel Arguments
    OCL_CHECK(err, err = m_kernel.setArg(0, m_buffer_instr));
    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_instr);
    sendBuffer(l_buffers);
}

void CGKernelControl::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_instr);
    getBuffer(l_buffers);
}

CGKernelStoreApk::CGKernelStoreApk(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelStoreApk::setMem(void* p_pk, unsigned int p_pkSize, void* p_Apk, unsigned int p_ApkSize) {
    cl_int err;
    // Running the kernel
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize);
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize);

    // Setting Kernel Arguments
    OCL_CHECK(err, err = m_kernel.setArg(1, m_buffer_pk));
    OCL_CHECK(err, err = m_kernel.setArg(2, m_buffer_Apk));

    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_pk);
    sendBuffer(l_buffers);
}

CGKernelUpdatePk::CGKernelUpdatePk(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelUpdatePk::setMem(void* p_pk, unsigned int p_pkSize, void* p_zk, unsigned int p_zkSize) {
    cl_int err;
    // Running the kernel
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize);
    m_buffer_zk = createDeviceBuffer(CL_MEM_READ_WRITE, p_zk, p_zkSize);

    // Setting Kernel Arguments
    int n_arg = 0;
    OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_pk));
    OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_pk));
    OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_zk));

    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_zk);
    l_buffers.push_back(m_buffer_pk);
    sendBuffer(l_buffers);
}
void CGKernelUpdatePk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_pk);
    getBuffer(l_buffers);
}

CGKernelUpdateRkJacobi::CGKernelUpdateRkJacobi(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelUpdateRkJacobi::setMem(void* p_rk,
                                    unsigned int p_rkSize,
                                    void* p_zk,
                                    unsigned int p_zkSize,
                                    void* p_jacobi,
                                    unsigned int p_jacobiSize,
                                    void* p_Apk,
                                    unsigned int p_ApkSize) {
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_rk = createDeviceBuffer(CL_MEM_READ_WRITE, p_rk, p_rkSize);
    m_buffer_zk = createDeviceBuffer(CL_MEM_READ_WRITE, p_zk, p_zkSize);
    m_buffer_jacobi = createDeviceBuffer(CL_MEM_READ_ONLY, p_jacobi, p_jacobiSize);
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize);

    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_zk);
    l_buffers.push_back(m_buffer_Apk);
    l_buffers.push_back(m_buffer_jacobi);

    // Setting Kernel Arguments
    int l_index = 0;
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_rk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_rk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_zk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_jacobi));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_Apk));

    // Copy input data to device global memory
    sendBuffer(l_buffers);
}
void CGKernelUpdateRkJacobi::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_zk);
    getBuffer(l_buffers);
}

CGKernelUpdateRk::CGKernelUpdateRk(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelUpdateRk::setMem(void* p_rk, unsigned int p_rkSize, void* p_Apk, unsigned int p_ApkSize) {
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_rk = createDeviceBuffer(CL_MEM_READ_WRITE, p_rk, p_rkSize);
    m_buffer_Apk = createDeviceBuffer(CL_MEM_READ_WRITE, p_Apk, p_ApkSize);

    l_buffers.push_back(m_buffer_rk);
    l_buffers.push_back(m_buffer_Apk);

    // Setting Kernel Arguments
    int l_index = 0;
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_rk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_rk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_Apk));

    // Copy input data to device global memory
    sendBuffer(l_buffers);
}
void CGKernelUpdateRk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_rk);
    getBuffer(l_buffers);
}

CGKernelUpdateXk::CGKernelUpdateXk(FPGA* p_fpga) : Kernel(p_fpga) {}
void CGKernelUpdateXk::setMem(void* p_xk, unsigned int p_xkSize, void* p_pk, unsigned int p_pkSize) {
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    // Running the kernel
    m_buffer_xk = createDeviceBuffer(CL_MEM_READ_WRITE, p_xk, p_xkSize);
    m_buffer_pk = createDeviceBuffer(CL_MEM_READ_WRITE, p_pk, p_pkSize);

    l_buffers.push_back(m_buffer_xk);
    l_buffers.push_back(m_buffer_pk);

    // Setting Kernel Arguments
    int l_index = 0;
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_xk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_xk));
    OCL_CHECK(err, err = m_kernel.setArg(l_index++, m_buffer_pk));

    // Copy input data to device global memory
    sendBuffer(l_buffers);
}
void CGKernelUpdateXk::getMem() {
    // Copy Result from Device Global Memory to Host Local Memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer_xk);
    getBuffer(l_buffers);
}

KernelLoadNnz::KernelLoadNnz(FPGA* p_fpga) : Kernel(p_fpga) {}
void KernelLoadNnz::setMem(std::vector<void*>& p_sigBuf, std::vector<unsigned int>& p_sigBufBytes) {
    cl_int err;
    unsigned int l_numChannels = p_sigBuf.size();
    m_buffers.resize(l_numChannels);
    std::vector<cl::Memory> l_buffers;
    for (unsigned int i = 0; i < l_numChannels; ++i) {
        m_buffers[i] = createDeviceBuffer(CL_MEM_READ_ONLY, p_sigBuf[i], p_sigBufBytes[i]);
        l_buffers.push_back(m_buffers[i]);
        OCL_CHECK(err, err = m_kernel.setArg(i, m_buffers[i]));
    }
    // Copy input data to device global memory
    sendBuffer(l_buffers);
}
KernelLoadCol::KernelLoadCol(FPGA* p_fpga) : Kernel(p_fpga) {}
void KernelLoadCol::setParParamMem(void* p_paramBuf, unsigned int p_paramBufSize) {
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    m_buffers[0] = createDeviceBuffer(CL_MEM_READ_ONLY, p_paramBuf, p_paramBufSize);
    OCL_CHECK(err, err = m_kernel.setArg(0, m_buffers[0]));
    l_buffers.push_back(m_buffers[0]);
    // Copy input data to device global memory
    sendBuffer(l_buffers);
}
void KernelLoadCol::setXMem(void* p_xBuf, unsigned int p_xBufSize) {
    cl_int err;
    std::vector<cl::Memory> l_buffers;
    m_buffers[1] = createDeviceBuffer(CL_MEM_READ_ONLY, p_xBuf, p_xBufSize);
    OCL_CHECK(err, err = m_kernel.setArg(1, m_buffers[1]));
    l_buffers.push_back(m_buffers[1]);
    // Copy input data to device global memory
    sendBuffer(l_buffers);
}

KernelLoadRbParam::KernelLoadRbParam(FPGA* p_fpga) : Kernel(p_fpga) {}
void KernelLoadRbParam::setMem(void* p_buf, unsigned int p_bufSize) {
    cl_int err;
    m_buffer = createDeviceBuffer(CL_MEM_READ_ONLY, p_buf, p_bufSize);
    OCL_CHECK(err, err = m_kernel.setArg(0, m_buffer));
    // Copy input data to device global memory
    std::vector<cl::Memory> l_buffers;
    l_buffers.push_back(m_buffer);
    sendBuffer(l_buffers);
}

xCgHost::xCgHost(int p_devId, std::string p_xclbinName) {
    init(p_devId, p_xclbinName);
};
void xCgHost::init(int p_devId, std::string p_xclbinName) {
    m_card.init(p_devId, p_xclbinName);
    m_krnCtl.fpga(&m_card);
    m_krnLoadArbParam.fpga(&m_card);
    m_krnLoadAval.fpga(&m_card);
    m_krnLoadPkApar.fpga(&m_card);
    m_krnStoreApk.fpga(&m_card);
    m_krnUpdatePk.fpga(&m_card);
    m_krnUpdateRkJacobi.fpga(&m_card);
    m_krnUpdateXk.fpga(&m_card);
    m_krnCtl.getCU("krnl_control");
    m_krnLoadAval.getCU("krnl_loadAval:{krnl_loadAval}");
    m_krnLoadPkApar.getCU("krnl_loadPkApar:{krnl_loadPkApar}");
    m_krnLoadArbParam.getCU("krnl_loadArbParam:{krnl_loadArbParam}");
    m_krnStoreApk.getCU("krnl_storeApk:{krnl_storeApk}");
    m_krnUpdatePk.getCU("krnl_update_pk");
    m_krnUpdateRkJacobi.getCU("krnl_update_rk_jacobi");
    m_krnUpdateXk.getCU("krnl_update_xk");
}
void xCgHost::sendMatDat(std::vector<void*>& p_nnzVal,
                         std::vector<unsigned int>& p_nnzValSize,
                         void* p_rbParam,
                         unsigned int p_rbParamSize,
                         void* p_parParam,
                         unsigned int p_parParamSize) {
    m_krnLoadAval.setMem(p_nnzVal, p_nnzValSize);
    m_krnLoadArbParam.setMem(p_rbParam, p_rbParamSize);
    m_krnLoadPkApar.setParParamMem(p_parParam, p_parParamSize);
}
void xCgHost::sendVecDat(void* p_pk,
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
    m_krnLoadPkApar.setXMem(p_pk, p_pkSize);
    m_krnStoreApk.setMem(p_pk, p_pkSize, p_Apk, p_ApkSize);
    m_krnUpdatePk.setMem(p_pk, p_pkSize, p_zk, p_zkSize);
    m_krnUpdateRkJacobi.setMem(p_rk, p_rkSize, p_zk, p_zkSize, p_jacobi, p_jacobiSize, p_Apk, p_ApkSize);
    m_krnUpdateXk.setMem(p_xk, p_xkSize, p_pk, p_pkSize);
}
void xCgHost::sendInstr(void* p_instr, unsigned int p_instrSize) {
    m_krnCtl.setMem(p_instr, p_instrSize);
}
void xCgHost::run() {
    m_krnCtl.enqueueTask();
    m_krnLoadArbParam.enqueueTask();
    m_krnLoadAval.enqueueTask();
    m_krnLoadPkApar.enqueueTask();
    m_krnStoreApk.enqueueTask();
    m_krnUpdatePk.enqueueTask();
    m_krnUpdateXk.enqueueTask();
    m_krnUpdateRkJacobi.enqueueTask();
}
void xCgHost::getDat() {
    m_krnCtl.getMem();
    m_krnUpdateXk.getMem();
    m_krnUpdateRkJacobi.getMem();
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

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

#ifndef CGHOST_HPP
#define CGHOST_HPP

#include "xFpga.hpp"

namespace xilinx_apps {
namespace pcg {

class CGKernelControl : public Kernel {
   public:
    CGKernelControl(FPGA* p_fpga = nullptr);
    bool setMem(void* p_instr, unsigned int p_instrBytes);
    bool getMem();

   private:
    cl::Buffer m_buffer_instr;
};

class CGKernelStoreApk : public Kernel {
   public:
    CGKernelStoreApk(FPGA* p_fpga = nullptr);
    bool setMem(void* p_pk, unsigned int p_pkSize, void* p_Apk, unsigned int p_ApkSize);

   private:
    cl::Buffer m_buffer_pk, m_buffer_Apk;
};

class CGKernelUpdatePk : public Kernel {
   public:
    CGKernelUpdatePk(FPGA* p_fpga = nullptr);
    bool setMem(void* p_pk, unsigned int p_pkSize, void* p_zk, unsigned int p_zkSize);
    bool getMem();

   private:
    cl::Buffer m_buffer_pk, m_buffer_zk;
};
class CGKernelUpdateRkJacobi : public Kernel {
   public:
    CGKernelUpdateRkJacobi(FPGA* p_fpga = nullptr);
    bool setMem(void* p_rk,
                unsigned int p_rkSize,
                void* p_zk,
                unsigned int p_zkSize,
                void* p_jacobi,
                unsigned int p_jacobiSize,
                void* p_Apk,
                unsigned int p_ApkSize);
    bool getMem();

   private:
    cl::Buffer m_buffer_Apk, m_buffer_rk, m_buffer_zk, m_buffer_jacobi;
};
class CGKernelUpdateRk : public Kernel {
   public:
    CGKernelUpdateRk(FPGA* p_fpga = nullptr);
    bool setMem(void* p_rk, unsigned int p_rkSize, void* p_Apk, unsigned int p_ApkSize);
    bool getMem();

   private:
    cl::Buffer m_buffer_Apk, m_buffer_rk;
};
class CGKernelUpdateXk : public Kernel {
   public:
    CGKernelUpdateXk(FPGA* p_fpga = nullptr);
    bool setMem(void* p_xk, unsigned int p_xkSize, void* p_pk, unsigned int p_pkSize);
    bool getMem();

   private:
    cl::Buffer m_buffer_xk, m_buffer_pk;
};

class KernelLoadNnz : public Kernel {
   public:
    KernelLoadNnz(FPGA* p_fpga = nullptr);
    bool setMem(std::vector<void*>& p_sigBuf, std::vector<unsigned int>& p_sigBufBytes);

   private:
    std::vector<cl::Buffer> m_buffers;
};
class KernelLoadCol : public Kernel {
   public:
    KernelLoadCol(FPGA* p_fpga = nullptr);
    // void setMem(void* p_paramBuf, unsigned int p_paramBufSize, void* p_xBuf, unsigned int p_xBufSize);
    bool setParParamMem(void* p_xBuf, unsigned int p_xBufSize);
    bool setXMem(void* p_xBuf, unsigned int p_xBufSize);

   private:
    cl::Buffer m_buffers[2];
};
class KernelLoadRbParam : public Kernel {
   public:
    KernelLoadRbParam(FPGA* p_fpga = nullptr);
    bool setMem(void* p_buf, unsigned int p_bufSize);

   private:
    cl::Buffer m_buffer;
};

class xCgHost {
   public:
    xCgHost(){};
    xCgHost(std::string p_xclbinName);
    bool init(std::string p_xclbinName);

    bool sendMatDat(std::vector<void*>& p_nnzVal,
                    std::vector<unsigned int>& p_nnzValSize,
                    void* p_rbParam,
                    unsigned int p_rbParamSize,
                    void* p_parParam,
                    unsigned int p_parParamSize);

    bool sendVecDat(void* p_pk,
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
                    unsigned int p_xkSize);

    bool sendInstr(void* p_instr, unsigned int p_instrSize);
    bool run();
    bool getDat();
    void finish();

   private:
    FPGA m_card;
    CGKernelControl m_krnCtl;
    KernelLoadNnz m_krnLoadAval;
    KernelLoadCol m_krnLoadPkApar;
    KernelLoadRbParam m_krnLoadArbParam;
    CGKernelStoreApk m_krnStoreApk;
    CGKernelUpdatePk m_krnUpdatePk;
    CGKernelUpdateRkJacobi m_krnUpdateRkJacobi;
    CGKernelUpdateXk m_krnUpdateXk;
};
}
}
#endif

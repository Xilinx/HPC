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

#ifndef BASICHOST_HPP
#define BASICHOST_HPP

#include "sw/xNativeFPGA.hpp"
#include "basicException.hpp"

template <unsigned int t_NetDataBits>
class basicHost {
   public:
    static const unsigned int t_NetBytes = t_NetDataBits / 8;

   public:
    basicHost() {}
    void init(xilinx_apps::hpc_common::FPGA* p_fpga) {
        m_card = p_fpga;
        m_krnS2mm.fpga(m_card);
        m_krnmm2S.fpga(m_card);
        m_krnCount.fpga(m_card);
    }
    void createKrnCount() { m_krnCount.createKernel("krnl_counter:{krnl_counter_0}"); }
    void createCountBufs(const size_t p_bytes) {
        void* l_outBuf = m_krnCount.createBO(2, p_bytes);
        m_krnCount.setMemArg(2);
        m_krnCountBufs.insert({2, l_outBuf});
    }

    void runKrnCount() {
        m_krnCount.run();
    }

    void* getCountRes() {
        m_krnCount.getBO(2);
        void* l_outBuf = m_krnCountBufs.find(2)->second;
        return l_outBuf;
    }
    void createKrnS2mm() { m_krnS2mm.createKernel("krnl_s2mm:{krnl_s2mm_0}"); }
    void createS2mmBufs() {
        size_t l_bufBytes = 128*1024;
        void* l_outBuf = m_krnS2mm.createBO(1, l_bufBytes);
        m_krnS2mm.setMemArg(1);
        m_krnS2mmBufs.insert({1, l_outBuf});
    }

    void runKrnS2mm(const size_t p_bytes) {
        if (p_bytes % t_NetBytes != 0) {
            throw basicInvalidValue("argument 2 of kernel krnl_s2mm must be multiple of " + std::to_string(t_NetBytes));
        }
        unsigned int l_netWords = p_bytes / t_NetBytes;
        m_krnS2mm.setScalarArg(2, (unsigned int)l_netWords);
        m_krnS2mm.run();
    }

    void* getS2mmRes() {
        m_krnS2mm.getBO(1);
        void* l_outBuf = m_krnS2mmBufs.find(1)->second;
        return l_outBuf;
    }
    void createKrnmm2S() { m_krnmm2S.createKernel("krnl_mm2s:{krnl_mm2s_0}"); }


    void runKrnmm2S(const size_t p_bytes) {
        if (p_bytes % t_NetBytes != 0) {
            throw basicInvalidValue("argument 2 of kernel krnl_mm2s must be multiple of " + std::to_string(t_NetBytes));
        }
        unsigned int l_netWords = p_bytes / t_NetBytes;
        m_krnmm2S.setScalarArg(2, (unsigned int)l_netWords);
        m_krnmm2S.run();
        m_krnmm2S.wait();
    }

   private:
    xilinx_apps::hpc_common::FPGA* m_card;
    xilinx_apps::hpc_common::KERNEL m_krnS2mm;
    xilinx_apps::hpc_common::KERNEL m_krnmm2S;
    xilinx_apps::hpc_common::KERNEL m_krnCount;
    std::map<const int, void*> m_krnS2mmBufs;
    std::map<const int, void*> m_krnCountBufs;
};

#endif

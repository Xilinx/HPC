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

#ifndef ECHOHOST_HPP
#define ECHOHOST_HPP

#include "sw/xNativeFPGA.hpp"
#include "basicException.hpp"

template <unsigned int t_NetDataBits>
class echoHost {
   public:
    static const unsigned int t_NetBytes = t_NetDataBits / 8;

   public:
    echoHost() {}
    void init(xilinx_apps::hpc_common::FPGA* p_fpga) {
        m_card = p_fpga;
        m_krnEcho.fpga(m_card);
    }
    void createKrnEcho() { m_krnEcho.createKernel("krnl_echo:{krnl_echo_0}"); }
    void createEchoBufs() {
        void* l_outBuf = m_krnEcho.createBO(1, sizeof(uint32_t));
        m_krnEcho.setMemArg(1);
        m_krnEchoBufs.insert({1, l_outBuf});
    }

    void runKrnEcho(const size_t p_bytes) {
        if (p_bytes % t_NetBytes != 0) {
            throw basicInvalidValue("argument 2 of kernel krnl_echo must be multiple of " + std::to_string(t_NetBytes));
        }
        unsigned int l_netWords = p_bytes / t_NetBytes;
        m_krnEcho.setScalarArg(2, (unsigned int)l_netWords);
        m_krnEcho.run();
    }

    void* getEchoRes() {
        m_krnEcho.getBO(1);
        void* l_outBuf = m_krnEchoBufs.find(1)->second;
        return l_outBuf;
    }

    void* finish() {
        m_krnEcho.wait();
    }

   private:
    xilinx_apps::hpc_common::FPGA* m_card;
    xilinx_apps::hpc_common::KERNEL m_krnEcho;
    std::map<const int, void*> m_krnEchoBufs;
};

#endif

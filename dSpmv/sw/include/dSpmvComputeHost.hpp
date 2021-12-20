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

#ifndef XILINX_APPS_DSPMVCOMPUTEHOST_HPP
#define XILINX_APPS_DSPMVCOMPUTEHOST_HPP

#include "sw/xNativeFPGA.hpp"
#include "sw/fp64/spmException.hpp"

namespace xilinx_apps {
namespace dspmv {

template <unsigned int t_NumChannels>
class dSpmvComputeHost {
    public:
        dSpmvComputeHost(){}
        void init(xilinx_apps::hpc_common::FPGA* p_fpga) {
            m_card = p_fpga;
            m_krnLoadNnz.fpga(m_card);
            m_krnLoadParX.fpga(m_card);
            m_krnLoadRbParam.fpga(m_card);
            m_krnTransY.fpga(m_card);
        }
        void createKernels() {
            m_krnLoadNnz.createKernel("loadNnzKernel:{krnl_loadNnz}");
            m_krnLoadParX.createKernel("loadParXkernel:{krnl_loadParX}");
            m_krnLoadRbParam.createKernel("loadRbParamKernel:{krnl_loadRbParam}");
            m_krnTransY.createKernel("krnl_transY:{krnl_transY_0}");
        }
        void createLoadNnzBufs(const size_t p_bytes[t_NumChannels], void* p_hostPtr[t_NumChannels]) {
            for (unsigned int i=0; i<t_NumChannels; ++i) {
                m_krnLoadNnz.createBOfromHostPtr(i, p_bytes[i], p_hostPtr[i]);
                m_krnLoadNnz.setMemArg(i);
                m_krnLoadNnzBufs.insert({i, p_hostPtr[i]});
            }
        }
        void createLoadParXbufs(const size_t p_bytes[2], void* p_hostPtr[2]) {
            for (unsigned int i=0; i<2; ++i) {
                m_krnLoadParX.createBOfromHostPtr(i, p_bytes[i], p_hostPtr[i]);
                m_krnLoadParX.setMemArg(i);
                m_krnLoadParXbufs.insert({i, p_hostPtr[i]});
            }
        }
        void createLoadRbParamBufs(const size_t p_bytes, void* p_hostPtr) {
            m_krnLoadRbParam.createBOfromHostPtr(0, p_bytes, p_hostPtr);
            m_krnLoadRbParam.setMemArg(0);
            m_krnLoadRbParamBufs.insert({0, p_hostPtr});
        }
        void setKrnTransYargs(const unsigned int p_startAddr, const unsigned int p_rows) {
            m_krnTransY.setScalarArg(0, p_startAddr);
            m_krnTransY.setScalarArg(1, p_rows);
        }
        void sendBOs() {
            for (unsigned int i=0; i<t_NumChannels; ++i) {
                m_krnLoadNnz.sendBO(i);
            }
            for (unsigned int i=0; i<2; ++i) {
                m_krnLoadParX.sendBO(i);
            }
            m_krnLoadRbParam.sendBO(0);
        }
        void run() {
            m_krnLoadNnz.run();
            m_krnLoadParX.run();
            m_krnLoadRbParam.run();
            m_krnTransY.run();
        }
        void finish() {
            m_krnLoadNnz.wait();
            m_krnLoadParX.wait();
            m_krnLoadRbParam.wait();
            m_krnTransY.wait();
        }
    private:
        xilinx_apps::hpc_common::FPGA* m_card;
        xilinx_apps::hpc_common::KERNEL m_krnLoadNnz;
        xilinx_apps::hpc_common::KERNEL m_krnLoadParX;
        xilinx_apps::hpc_common::KERNEL m_krnLoadRbParam;
        xilinx_apps::hpc_common::KERNEL m_krnTransY;
        std::map<const int, void*> m_krnLoadNnzBufs;
        std::map<const int, void*> m_krnLoadParXbufs;
        std::map<const int, void*> m_krnLoadRbParamBufs;
};

}
}

#endif

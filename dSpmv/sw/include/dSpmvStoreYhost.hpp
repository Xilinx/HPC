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

#ifndef XILINX_APPS_DSPMVSTOREYHOST_HPP
#define XILINX_APPS_DSPMVSTOREYHOST_HPP

#include "sw/xNativeFPGA.hpp"
#include "sw/fp64/spmException.hpp"

namespace xilinx_apps {
namespace dspmv {

template <unsigned int t_NumChannels>
class dSpmvStoreYhost {
    public:
        dSpmvStoreYhost(){}
        void init(xilinx_apps::hpc_common::FPGA* p_fpga) {
            m_card = p_fpga;
            m_krnStoreY.fpga(m_card);
        }
        void createKernels() {
            m_krnStoreY.createKernel("krnl_dStoreY:{krnl_dStoreY_0}");
        }
        void createStoreYbufs(const size_t p_bytes, void* p_hostPtr) {
            m_krnStoreY.createBOfromHostPtr(2, p_bytes, p_hostPtr);
            m_krnStoreY.setMemArg(2);
            m_krnStoreYbufs.insert({2, p_hostPtr});
        }
        void setStoreYargs(const unsigned int p_numComputeNodes) {
            m_krnStoreY.setScalarArg(0, p_numComputeNodes);
        }
        void run() {
            m_krnStoreY.run();
        }
        void getY() {
            m_krnStoreY.getBO(2);
        }
    private:
        xilinx_apps::hpc_common::FPGA* m_card;
        xilinx_apps::hpc_common::KERNEL m_krnStoreY;
        std::map<const int, void*> m_krnStoreYbufs;
};

}
}

#endif

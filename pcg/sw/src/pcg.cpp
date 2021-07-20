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

#include "pcg.h"
#include "impl/pcgImp.hpp"

using PcgImpl = xilinx_apps::pcg::PCGImpl<double, 4, 64, 8, 16, 4096, 4096, 256>;

extern "C" {

void* create_JPCG_handle(int deviceId, const char* xclbinPath) {
    PcgImpl* pImpl = new PcgImpl();
    pImpl->init(deviceId, xclbinPath);
    return pImpl;
}

void destroy_JPCG_handle(void* handle) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    delete pImpl;
}

void JPCG_coo(void* handle,
              JPCG_Mode mode,
              uint32_t p_n,
              uint32_t p_nnz,
              uint32_t* p_rowIdx,
              uint32_t* p_colIdx,
              double* p_data,
              double* matJ,
              double* b,
              double* x,
              uint32_t p_maxIter,
              double p_tol,
              uint32_t* p_iter,
              double* p_res) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    switch (mode) {
        case JPCG_MODE_FULL:
            pImpl->setCooMat(p_n, p_nnz, p_rowIdx, p_colIdx, p_data);
            break;
        case JPCG_MODE_KEEP_NZ_LAYOUT:
            pImpl->updateMat(p_n, p_nnz, p_data);
            break;
        default:
            break;
    }
    pImpl->setVec(p_n, b, matJ);
    xilinx_apps::pcg::Results<double> l_res = pImpl->run(p_maxIter, p_tol);
    *p_res = std::sqrt(l_res.m_residual / pImpl->getDot());
    *p_iter = l_res.m_nIters;
}
}

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

#include <chrono>
#include "pcg.h"
#include "impl/pcgImp.hpp"

using PcgImpl = xilinx_apps::pcg::PCGImpl<double, 4, 64, 8, 16, 4096, 4096, 256>;

extern "C" {

XJPCG_Status_t create_JPCG_handle(void **handle, const int deviceId, const char* xclbinPath) {
    auto last = std::chrono::high_resolution_clock::now();
    PcgImpl* pImpl = new PcgImpl();
    pImpl->init(deviceId, xclbinPath);
    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - last;
    pImpl->getMetrics()->m_init = duration.count();
    *handle = pImpl;
    return XJPCG_STATUS_SUCCESS;
}

void destroy_JPCG_handle(void* handle) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    delete pImpl;
}

XJPCG_Status_t xJPCG_coo(void* handle,
              const uint32_t p_n,
              const uint32_t p_nnz,
              const uint32_t* p_rowIdx,
              const uint32_t* p_colIdx,
              const double* p_data,
              const double* matJ,
              const double* b,
              const double* x,
              const uint32_t p_maxIter,
              const double p_tol,
              uint32_t* p_iter,
              double* p_res,
              const XJPCG_Mode mode) {
    auto last = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration;

    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    switch (mode & XJPCG_MODE_KEEP_MATRIX) {
        case XJPCG_MODE_DEFAULT:
            pImpl->setCooMat(p_n, p_nnz, p_rowIdx, p_colIdx, p_data);
            break;
        case XJPCG_MODE_KEEP_NZ_LAYOUT:
            pImpl->updateMat(p_n, p_nnz, p_data);
            break;
        default:
            break;
    }

    duration = std::chrono::high_resolution_clock::now() - last;
    last = std::chrono::high_resolution_clock::now();
    pImpl->getMetrics()->m_matProc = duration.count();

    pImpl->setVec(p_n, b, matJ);
    duration = std::chrono::high_resolution_clock::now() - last;
    last = std::chrono::high_resolution_clock::now();
    pImpl->getMetrics()->m_vecProc = duration.count();

    xilinx_apps::pcg::Results<double> l_res = pImpl->run(p_maxIter, p_tol);
    *p_res = std::sqrt(l_res.m_residual / pImpl->getDot());
    *p_iter = l_res.m_nIters;
    memcpy((char*) x, (char*) l_res.m_x, sizeof(double) * p_n);
    duration = std::chrono::high_resolution_clock::now() - last;
    last = std::chrono::high_resolution_clock::now();
    pImpl->getMetrics()->m_solver = duration.count();
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_getMetrics(void* handle, XJPCG_Metric_t *metric) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    *metric = *pImpl->getMetrics();
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_peekAtLastStatus(void* handle) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    return pImpl->getLastStatus();
}

const char* xJPCG_getLastMessage(void* handle) {
    auto pImpl = reinterpret_cast<PcgImpl*>(handle);
    return pImpl->getLastMessage().c_str();
}
}

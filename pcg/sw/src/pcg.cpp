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
#include <cstdlib>
#include <ctime>
#include "pcg.h"
#include "impl/pcgImp.hpp"

using PcgImpl = xilinx_apps::pcg::PCGImpl<double, 4, 64, 8, 16, 4096, 4096, 256>;

namespace {

const int secretCode() {
    srand(time(NULL));
    static const int val = std::rand();
    return val;
}
double getDuration(std::chrono::time_point<std::chrono::high_resolution_clock> &last){
        std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - last;
        last = std::chrono::high_resolution_clock::now();
        return duration.count();
}

void checkHandle(const XJPCG_Handle_t *handle) {
    assert(handle != nullptr);
    if(handle -> pcg == nullptr || handle -> code != secretCode())
        throw xilinx_apps::pcg::CgException("Handle is not correctly initialized.", XJPCG_STATUS_NOT_INITIALIZED);
}

XJPCG_Status_t setStatusMessage(XJPCG_Handle_t *handle, XJPCG_Status_t p_stat, const char* p_str){
    assert(handle != nullptr);
    handle -> status = p_stat;
    handle -> msg = p_str;
    return p_stat;
}

}

extern "C" {

XJPCG_Status_t xJPCG_createHandle(XJPCG_Handle_t *handle, const int deviceId, const char* xclbinPath) {
    assert(handle != nullptr);
    try {
        auto last = std::chrono::high_resolution_clock::now();
        PcgImpl * pImpl = new PcgImpl();
        handle -> pcg = pImpl;
        handle -> code = secretCode();
        checkHandle(handle);
        pImpl->init(deviceId, xclbinPath);
        pImpl->getMetrics()->m_init = getDuration(last);
    } catch (const xilinx_apps::pcg::CgException & err) {
        return setStatusMessage(handle, err.getStatus(), err.what());
    } catch (const std::exception & err ) {
        return setStatusMessage(handle, XJPCG_STATUS_INTERNAL_ERROR, err.what());
    } 
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_destroyHandle(XJPCG_Handle_t *handle) {
    assert(handle != nullptr);
    try {
        checkHandle(handle);
        auto pImpl = reinterpret_cast<PcgImpl*>(handle -> pcg);
        delete pImpl;
    } catch (const xilinx_apps::pcg::CgException & err) {
        return setStatusMessage(handle, err.getStatus(), err.what());
    } catch (const std::exception & err ) {
        return setStatusMessage(handle, XJPCG_STATUS_INTERNAL_ERROR, err.what());
    } 
    return XJPCG_STATUS_SUCCESS;
}


XJPCG_Status_t xJPCG_cooSolver(XJPCG_Handle_t *handle,
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
    assert(handle != nullptr);
    try {
        checkHandle(handle);
        auto pImpl = reinterpret_cast<PcgImpl*>(handle -> pcg);
        auto last = std::chrono::high_resolution_clock::now();

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

        pImpl->getMetrics()->m_matProc = getDuration(last);

        pImpl->setVec(p_n, b, matJ);
        pImpl->getMetrics()->m_vecProc = getDuration(last);

        xilinx_apps::pcg::Results<double> l_res = pImpl->run(p_maxIter, p_tol);
        *p_res = std::sqrt(l_res.m_residual / pImpl->getDot());
        *p_iter = l_res.m_nIters;
        memcpy((char*) x, (char*) l_res.m_x, sizeof(double) * p_n);
        pImpl->getMetrics()->m_solver = getDuration(last);
        if(*p_iter > p_maxIter || *p_res > p_tol) {
            throw xilinx_apps::pcg::CgException("Solver is not convergent.",XJPCG_STATUS_EXECUTION_FAILED);
        }
    } catch (const xilinx_apps::pcg::CgException & err) {
        return setStatusMessage(handle, err.getStatus(), err.what());
    } catch (const std::exception & err ) {
        return setStatusMessage(handle, XJPCG_STATUS_INTERNAL_ERROR, err.what());
    } 
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_getMetrics(XJPCG_Handle_t *handle, XJPCG_Metric_t *metric) {
    assert(handle != nullptr);
    try {
        checkHandle(handle);
        auto pImpl = reinterpret_cast<PcgImpl*>(handle -> pcg);
        *metric = *pImpl->getMetrics();
    } catch (const xilinx_apps::pcg::CgException & err) {
        return setStatusMessage(handle, err.getStatus(), err.what());
    } catch (const std::exception & err ) {
        return setStatusMessage(handle, XJPCG_STATUS_INTERNAL_ERROR, err.what());
    } 
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Handle_t *handle) {
    assert(handle != nullptr);
    return handle -> status; 
}

const char* xJPCG_getLastMessage(const XJPCG_Handle_t *handle) {
    assert(handle != nullptr);
    return handle -> msg;
}

const char* XJPCG_getErrorString(const XJPCG_Status_t code){ 
    switch(code) {
        case XJPCG_STATUS_SUCCESS:
            return "XJPCG_STATUS_SUCCESS";
        case XJPCG_STATUS_NOT_INITIALIZED: 
            return "XJPCG_STATUS_NOT_INITIALIZED";
        case XJPCG_STATUS_ALLOC_FAILED:
            return "XJPCG_STATUS_ALLOC_FAILED";
        case XJPCG_STATUS_INVALID_VALUE:
            return "XJPCG_STATUS_INVALID_VALUE";
        case XJPCG_STATUS_EXECUTION_FAILED:
            return "XJPCG_STATUS_EXECUTION_FAILED";
        case XJPCG_STATUS_INTERNAL_ERROR:
            return "XJPCG_STATUS_INTERNAL_ERROR";
        case XJPCG_STATUS_NOT_SUPPORTED:
            return "XJPCG_STATUS_NOT_SUPPORTED";
    }
    return "Unknown Error Code!";
}

}

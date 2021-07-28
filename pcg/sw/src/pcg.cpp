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
#include <cassert>
#include "pcg.h"
#include "impl/pcgImp.hpp"

using PcgImpl = xilinx_apps::pcg::PCGImpl<double, 4, 64, 8, 16, 4096, 4096, 256>;

namespace {
double getDuration(std::chrono::time_point<std::chrono::high_resolution_clock>& last) {
    std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - last;
    last = std::chrono::high_resolution_clock::now();
    return duration.count();
}
}

extern "C" {

XJPCG_Status_t xJPCG_createHandle(XJPCG_Handle_t *handle, const int deviceId, const char* xclbinPath) {
    assert(handle != nullptr);
    handle->pcg = new PcgImpl();
    if (handle->pcg == nullptr)
        return XJPCG_STATUS_NOT_INITIALIZED;
    auto pImpl = reinterpret_cast<PcgImpl* >(handle->pcg);
    try{
        auto last = std::chrono::high_resolution_clock::now();
        pImpl->init(deviceId, xclbinPath);
        pImpl->getMetrics()->m_objInit = getDuration(last);
    } catch (const xilinx_apps::pcg::CgException& err) {
        return pImpl->setStatusMessage(err.getStatus(), err.what());
    } catch (const std::exception& err) {
        return pImpl->setStatusMessage(XJPCG_STATUS_INTERNAL_ERROR, err.what());
    }
    return pImpl->setStatusMessage(XJPCG_STATUS_SUCCESS, "API returns successfully");
}

XJPCG_Status_t xJPCG_destroyHandle(const XJPCG_Handle_t handle) {
    if (handle.pcg == nullptr)
        return XJPCG_STATUS_NOT_INITIALIZED;
    auto pImpl = reinterpret_cast<PcgImpl*>(handle.pcg);
    delete pImpl;
    return XJPCG_STATUS_SUCCESS;
}

XJPCG_Status_t xJPCG_cooSolver(const XJPCG_Handle_t handle,
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
    if (handle.pcg == nullptr)
        return XJPCG_STATUS_NOT_INITIALIZED;
    auto pImpl = reinterpret_cast<PcgImpl*>(handle.pcg);
    try {
        auto last = std::chrono::high_resolution_clock::now();
        bool first = pImpl->isFirstCall();
        switch (mode) {
            case XJPCG_MODE_DEFAULT:
                pImpl->setCooMat(p_n, p_nnz, p_rowIdx, p_colIdx, p_data);
                break;
            case XJPCG_MODE_KEEP_NZ_LAYOUT:
                if(first)
                    throw xilinx_apps::pcg::CgInvalidValue("wrong solver mode for the first call, please use XJPCG_MODEL_DEFAULT.");
                pImpl->updateMat(p_n, p_nnz, p_data);
                break;
            default:
                if(first)
                    throw xilinx_apps::pcg::CgInvalidValue("wrong solver mode for the first call, please use XJPCG_MODEL_DEFAULT.");
                break;
        }

        pImpl->getMetrics()->m_matProc = getDuration(last);
        pImpl->setVec(p_n, b, matJ);
        pImpl->getMetrics()->m_vecProc = getDuration(last);

        xilinx_apps::pcg::Results<double> l_res = pImpl->run(p_maxIter, p_tol);
        *p_res = std::sqrt(l_res.m_residual / pImpl->getDot());
        *p_iter = l_res.m_nIters;
        memcpy((char*)x, (char*)l_res.m_x, sizeof(double) * p_n);
        pImpl->getMetrics()->m_solver = getDuration(last);
        if(*p_res > p_tol) {
            throw xilinx_apps::pcg::CgExecutionFailed("exit with divergent solution after " + std::to_string(*p_iter) + " iterations.");
        }
    } catch (const xilinx_apps::pcg::CgException& err) {
        return pImpl->setStatusMessage(err.getStatus(), err.what());
    } catch (const std::exception& err) {
        return pImpl->setStatusMessage(XJPCG_STATUS_INTERNAL_ERROR, err.what());
    }
    return pImpl->setStatusMessage(XJPCG_STATUS_SUCCESS, "API returns successfully");
}

XJPCG_Status_t xJPCG_getMetrics(const XJPCG_Handle_t handle, XJPCG_Metric_t* metric) {
    if (handle.pcg == nullptr)
        return XJPCG_STATUS_NOT_INITIALIZED;
    auto pImpl = reinterpret_cast<PcgImpl*>(handle.pcg);
    *metric = *pImpl->getMetrics();
    return pImpl->setStatusMessage(XJPCG_STATUS_SUCCESS, "API returns successfully");
}

XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Handle_t handle) {
    if (handle.pcg == nullptr)
        return XJPCG_STATUS_NOT_INITIALIZED;
    auto pImpl = reinterpret_cast<PcgImpl*>(handle.pcg);
    return pImpl->getLastStatus();
}

const char* xJPCG_getLastMessage(const XJPCG_Handle_t handle) {
    if (handle.pcg == nullptr)
        return nullptr;
    auto pImpl = reinterpret_cast<PcgImpl*>(handle.pcg);
    return pImpl->getLastMessage().c_str();
}

const char* xJPCG_getErrorString(const XJPCG_Status_t code) {
    switch (code) {
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
        case XJPCG_STATUS_DYNAMIC_LOADING_ERROR:
            return "XJPCG_STATUS_DYNAMIC_LOADING_ERROR";
    }
    return "Unknown Error Code!";
}
}

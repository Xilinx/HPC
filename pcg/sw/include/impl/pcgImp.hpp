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

#ifndef PCG_IMP_HPP
#define PCG_IMP_HPP

#include "gen_signature.hpp"
#include "cgVector.hpp"
#include "cgHost.hpp"
#include "pcg.h"
#include "cgException.hpp"

namespace xilinx_apps {
namespace pcg {

template <typename t_DataType>
struct Results {
    void* m_x;
    t_DataType m_residual;
    unsigned int m_nIters;
};

template <typename t_DataType,
          unsigned int t_ParEntries,
          unsigned int t_InstrBytes,
          unsigned int t_AccLatency,
          unsigned int t_HbmChannels,
          unsigned int t_MaxRows,
          unsigned int t_MaxCols,
          unsigned int t_HbmMemBits>
class PCGImpl {
   public:
    PCGImpl(){};
    PCGImpl(std::string p_xclbinName) { m_host.init(p_xclbinName); }
    void init(std::string p_xclbinName) { m_host.init(p_xclbinName); }
    void setCooMat(const uint32_t p_dim,
                   const uint32_t p_nnz,
                   const uint32_t* p_rowIdx,
                   const uint32_t* p_colIdx,
                   const t_DataType* p_data,
                   const int p_storeType) {
        if (p_dim == 0) {
            throw CgInvalidValue("Wrong dimension size.");
        }
        if (p_nnz == 0) {
            throw CgInvalidValue("Wrong non-zero element size.");
        }
        if (p_rowIdx == nullptr || p_colIdx == nullptr || p_data == nullptr) {
            throw CgInvalidValue("Matrix is nullptr.");
        }
        m_matPar = m_spmPar.partitionCooMat(p_dim, p_dim, p_nnz, p_rowIdx, p_colIdx, p_data, p_storeType);
        bool l_send = m_host.sendMatDat(m_matPar.m_nnzValPtr, m_matPar.m_nnzValSize, m_matPar.m_rbParamPtr,
                                        m_matPar.m_rbParamSize, m_matPar.m_parParamPtr, m_matPar.m_parParamSize);
        if (l_send == false) {
            throw CgAllocFailed("Send matirx data failed.");
        }
    }
    template <typename t_IdxType>
    void setCscSymMat(const uint32_t p_dim,
                      const uint32_t p_nnz,
                      const t_IdxType* p_rowIdx,
                      const t_IdxType* p_colPtr,
                      const t_DataType* p_data,
                      const int p_storeType) {
        if (p_dim == 0) {
            throw CgInvalidValue("Wrong dimension size.");
        }
        if (p_nnz == 0) {
            throw CgInvalidValue("Wrong non-zero element size.");
        }
        if (p_rowIdx == nullptr || p_colPtr == nullptr || p_data == nullptr) {
            throw CgInvalidValue("Matrix is nullptr.");
        }
        uint32_t l_nnz = p_nnz * 2 - p_dim;
        m_matPar = m_spmPar.partitionCscSymMat(p_dim, l_nnz, p_rowIdx, p_colPtr, p_data, p_storeType);
        bool l_send = m_host.sendMatDat(m_matPar.m_nnzValPtr, m_matPar.m_nnzValSize, m_matPar.m_rbParamPtr,
                                        m_matPar.m_rbParamSize, m_matPar.m_parParamPtr, m_matPar.m_parParamSize);
        if (l_send == false) {
            throw CgAllocFailed("Send matirx data failed.");
        }
    }
    int updateMat(const uint32_t p_dim, const uint32_t p_nnz, const t_DataType* p_data) {
        if (p_dim == 0) {
            throw CgInvalidValue("Wrong dimension size.");
        }
        if (p_nnz == 0) {
            throw CgInvalidValue("Wrong non-zero element size.");
        }
        if (p_data == nullptr) {
            throw CgInvalidValue("Matrix is nullptr.");
        }
        if (m_spmPar.checkUpdateDim(p_dim, p_dim, p_nnz) == 0) {
            m_matPar = m_spmPar.updateMat(p_data);
            bool l_send = m_host.sendMatDat(m_matPar.m_nnzValPtr, m_matPar.m_nnzValSize, m_matPar.m_rbParamPtr,
                                            m_matPar.m_rbParamSize, m_matPar.m_parParamPtr, m_matPar.m_parParamSize);
            if (l_send == false) {
                throw CgAllocFailed("Send matirx data failed.");
            }
            return 0;
        } else {
            return -1;
        }
    }
    template <typename t_IdxType>
    int updateCscSymMat(const uint32_t p_dim,
                        const uint32_t p_nnz,
                        const t_IdxType* p_rowIdx,
                        const t_IdxType* p_colPtr,
                        const t_DataType* p_data,
                        const int p_storeType) {
        if (p_dim == 0) {
            throw CgInvalidValue("Wrong dimension size.");
        }
        if (p_nnz == 0) {
            throw CgInvalidValue("Wrong non-zero element size.");
        }
        if (p_data == nullptr) {
            throw CgInvalidValue("Matrix is nullptr.");
        }
        uint32_t l_nnz = p_nnz * 2 - p_dim;
        if (m_spmPar.checkUpdateDim(p_dim, p_dim, l_nnz) == 0) {
            m_matPar = m_spmPar.updateCscSymMat(p_dim, l_nnz, p_rowIdx, p_colPtr, p_data, p_storeType);
            bool l_send = m_host.sendMatDat(m_matPar.m_nnzValPtr, m_matPar.m_nnzValSize, m_matPar.m_rbParamPtr,
                                            m_matPar.m_rbParamSize, m_matPar.m_parParamPtr, m_matPar.m_parParamSize);
            if (l_send == false) {
                throw CgAllocFailed("Send matirx data failed.");
            }
            return 0;
        } else {
            return -1;
        }
    }

    void setVec(const uint32_t p_dim, const t_DataType* p_b, const t_DataType* p_diagA) {
        if (p_dim != m_genCgVec.getDim()) {
            m_genCgVec.loadVec(p_dim, p_b, p_diagA);
        } else {
            m_genCgVec.updateVec(p_dim, p_b, p_diagA);
        }
        m_genCgVec.init();
        this->sendVec();
    }

    Results<t_DataType> run(unsigned int p_maxIter, t_DataType p_tol) {
        this->setInstr(p_maxIter + 1, p_tol);
        bool l_run = m_host.run();
        if (l_run == false) {
            throw CgInternalError("Run kernel failed.");
        }
        CgVector l_resVec = this->getRes();
        Results<t_DataType> l_res;
        l_res.m_x = l_resVec.h_xk;
        xf::hpc::MemInstr<t_InstrBytes> l_memInstr;
        xf::hpc::cg::CGSolverInstr<CG_dataType> l_cgInstr;
        l_res.m_nIters = 0;
        l_res.m_residual = 0;

        CgInstr l_instr = m_genInstr.getInstrPtr();
        for (unsigned int i = 0; i <= p_maxIter; i++) {
            l_res.m_nIters = i;
            l_cgInstr.load((uint8_t*)(l_instr.h_instr) + (i + 1) * t_InstrBytes, l_memInstr);
            // std::cout << "l_cgInstr: " << std::scientific << l_cgInstr << std::endl;
            if (l_cgInstr.getMaxIter() == 0) {
                break;
            }
            l_res.m_residual = l_cgInstr.getRes();
        }
        m_firstCall = false;
        return l_res;
    }

    std::vector<uint32_t> getMatInfo() {
        std::vector<uint32_t> l_info(6);
        l_info[0] = m_matPar.m_m;
        l_info[1] = m_matPar.m_n;
        l_info[2] = m_matPar.m_nnz;
        l_info[3] = m_matPar.m_mPad;
        l_info[4] = m_matPar.m_nPad;
        l_info[5] = m_matPar.m_nnzPad;
        return l_info;
    }
    xf::sparse::MatPartition getMatPar() { return m_matPar; }
    t_DataType getDot() { return m_genCgVec.getDot(); }
    t_DataType getRz() { return m_genCgVec.getRz(); }
    CgVector getVec() { return m_genCgVec.getVec(); }

    CgVector getRes() {
        m_host.getDat();
        m_host.finish();
        return m_genCgVec.getVec();
    }

    void sendVec() {
        CgVector l_cgVec = m_genCgVec.getVec();
        bool l_send = m_host.sendVecDat(l_cgVec.h_pk, l_cgVec.vecBytes, l_cgVec.h_Apk, l_cgVec.vecBytes, l_cgVec.h_zk,
                                        l_cgVec.vecBytes, l_cgVec.h_rk, l_cgVec.vecBytes, l_cgVec.h_jacobi,
                                        l_cgVec.vecBytes, l_cgVec.h_xk, l_cgVec.vecBytes);
        if (l_send == false) {
            throw CgAllocFailed("Send vector data failed.");
        }
    }
    void setInstr(unsigned int p_maxIter, t_DataType p_tol) {
        t_DataType l_dot = m_genCgVec.getDot();
        t_DataType l_rz = m_genCgVec.getRz();
        m_genInstr.setInstr(p_maxIter, m_genCgVec.getDimAligned(), l_dot, p_tol, l_rz);
        m_genInstr.updateInstr();
        CgInstr l_cgInstr = m_genInstr.getInstrPtr();
        bool l_send = m_host.sendInstr(l_cgInstr.h_instr, l_cgInstr.h_instrBytes);
        if (l_send == false) {
            throw CgAllocFailed("Send Instruction data failed.");
        }
    }

    XJPCG_Metric_t* getMetrics() { return &m_Metrics; }
    const XJPCG_Metric_t* getMetrics() const { return &m_Metrics; }

    XJPCG_Status_t setStatusMessage(XJPCG_Status_t p_stat, const std::string p_str) const {
        m_lastStatus = p_stat;
        m_lastMessage = p_str;
        return m_lastStatus;
    }

    XJPCG_Status_t getLastStatus() const noexcept { return m_lastStatus; }

    const std::string& getLastMessage() const noexcept { return m_lastMessage; }
    bool isFirstCall() const { return m_firstCall; }

   private:
    mutable XJPCG_Status_t m_lastStatus;
    mutable std::string m_lastMessage;
    bool m_firstCall = true;

    xf::sparse::SpmPar<t_DataType> m_spmPar =
        xf::sparse::SpmPar<t_DataType>(t_ParEntries, t_AccLatency, t_HbmChannels, t_MaxRows, t_MaxCols, t_HbmMemBits);
    GenCgVector<t_DataType, t_ParEntries> m_genCgVec;
    GenCgInstr<t_DataType, t_InstrBytes> m_genInstr;
    xCgHost m_host;
    xf::sparse::MatPartition m_matPar;
    XJPCG_Metric_t m_Metrics;
};
}
}
#endif

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

#ifndef CGVECTOR_HPP
#define CGVECTOR_HPP

#include <vector>
#include "impl/cgInstr.hpp"
#include "utils.hpp"

using namespace xf::hpc;

namespace xilinx_apps {
namespace pcg {

struct CgInputVec {
    unsigned int vecBytes;
    void* h_b;
    void* h_diag;
};

struct CgVector {
    unsigned int vecBytes;
    void* h_Apk;
    void* h_jacobi;
    void* h_pk;
    void* h_rk;
    void* h_xk;
    void* h_zk;
};

struct CgInstr {
    unsigned int h_instrBytes;
    void* h_instr;
};

template <typename t_DataType, unsigned int t_ParEntries>
class GenCgVector {
   public:
    GenCgVector() : m_dim(0), m_dot(0), m_rz(0){};
    void loadVec(unsigned int p_dim, t_DataType* p_b, t_DataType* p_diagA) {
        m_diagA.clear();
        m_b.clear();
        m_Apk.clear();
        m_jacobi.clear();
        m_rk.clear();
        m_xk.clear();
        m_zk.clear();
        m_dim = p_dim;
        unsigned int l_dimAlignedBks = (p_dim + t_ParEntries - 1) / t_ParEntries;
        unsigned int l_dimAligned = l_dimAlignedBks * t_ParEntries;
        m_dimAligned = l_dimAligned;
        m_diagA.insert(m_diagA.end(), p_diagA, p_diagA + p_dim);
        m_diagA.insert(m_diagA.end(), m_dimAligned - m_dim, 1);
        m_b.insert(m_b.end(), p_b, p_b + p_dim);
        m_b.insert(m_b.end(), m_dimAligned - m_dim, 0);
        m_Apk.assign(m_dimAligned, 0);
        m_jacobi.assign(m_dimAligned, 1);
        m_pk.assign(m_dimAligned, 0);
        m_rk.assign(m_dimAligned, 0);
        m_xk.assign(m_dimAligned, 0);
        m_zk.assign(m_dimAligned, 0);
    }
    void updateVec(unsigned int p_dim, t_DataType* p_b, t_DataType* p_diagA) {
        std::copy(p_diagA, p_diagA + p_dim, m_diagA.begin());
        std::copy(p_b, p_b + p_dim, m_b.begin());
        std::fill(m_Apk.begin(), m_Apk.end(), 0);
        std::fill(m_jacobi.begin(), m_jacobi.end(), 1);
        std::fill(m_pk.begin(), m_pk.end(), 0);
        std::fill(m_rk.begin(), m_rk.end(), 0);
        std::fill(m_xk.begin(), m_xk.end(), 0);
        std::fill(m_zk.begin(), m_zk.end(), 0);
    }
    CgInputVec getInputVec() {
        CgInputVec l_res;
        l_res.vecBytes = m_dimAligned * sizeof(t_DataType);
        l_res.h_b = (void*)(m_b.data());
        l_res.h_diag = (void*)(m_diagA.data());
        return l_res;
    }
    void init() {
        m_dot = 0;
        m_rz = 0;
        m_xk.assign(m_dimAligned, 0);
        m_Apk.assign(m_dimAligned, 0);
        for (unsigned int i = 0; i < m_dimAligned; ++i) {
            m_rk[i] = m_b[i];
            m_jacobi[i] = 1.0 / m_diagA[i];
            m_zk[i] = m_jacobi[i] * m_rk[i];
            m_dot += m_b[i] * m_b[i];
            m_rz += m_rk[i] * m_zk[i];
            m_pk[i] = m_zk[i];
        }
    }
    CgVector getVec() {
        CgVector l_vec;
        l_vec.vecBytes = m_dimAligned * sizeof(t_DataType);
        l_vec.h_Apk = (void*)(m_Apk.data());
        l_vec.h_jacobi = (void*)(m_jacobi.data());
        l_vec.h_pk = (void*)(m_pk.data());
        l_vec.h_rk = (void*)(m_rk.data());
        l_vec.h_xk = (void*)(m_xk.data());
        l_vec.h_zk = (void*)(m_zk.data());
        return l_vec;
    }
    t_DataType getDot() { return m_dot; }
    t_DataType getRz() { return m_rz; }
    void* getXk() { return (void*)(m_xk.data()); }
    unsigned int getDimAligned() { return m_dimAligned; }
    unsigned int getDim() { return m_dim; }

   private:
    unsigned int m_dim, m_dimAligned;
    t_DataType m_dot, m_rz;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_diagA;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_b;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_Apk;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_jacobi;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_pk;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_rk;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_xk;
    std::vector<t_DataType, alignedAllocator<t_DataType> > m_zk;
};

template <typename t_DataType, unsigned int t_InstrBytes>
class GenCgInstr {
   public:
    GenCgInstr(){};
    void setInstr(
        unsigned int p_maxIter, unsigned int p_dimAligned, t_DataType p_dot, t_DataType p_tol, t_DataType p_rz) {
        unsigned int l_instrSize = t_InstrBytes * (1 + p_maxIter);
        m_instr.resize(l_instrSize);
        std::fill(m_instr.begin(), m_instr.end(), 0);
        m_cgInstr.setMaxIter(p_maxIter);
        m_cgInstr.setTols(p_dot * p_tol * p_tol);
        m_cgInstr.setRes(p_dot);
        m_cgInstr.setRZ(p_rz);
        m_cgInstr.setVecSize(p_dimAligned);
    }
    void updateInstr() {
        MemInstr<t_InstrBytes> l_memInstr;
        m_cgInstr.store(m_instr.data(), l_memInstr);
    }

    CgInstr getInstrPtr() {
        CgInstr l_res;
        l_res.h_instrBytes = m_instr.size();
        l_res.h_instr = (void*)(m_instr.data());
        return l_res;
    }

   private:
    std::vector<uint8_t, alignedAllocator<uint8_t> > m_instr;
    cg::CGSolverInstr<CG_dataType> m_cgInstr;
};
}
}
#endif

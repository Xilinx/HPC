/*
 * Copyright 2019 Xilinx, Inc.
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

#ifndef PCG_HPP
#define PCG_HPP

#include "gen_signature.hpp"
#include "cgVector.hpp"
#include "cgHost.hpp"

template <typename t_DataType,
          unsigned int t_ParEntries,
          unsigned int t_InstrBytes,
          unsigned int t_AccLatency,
          unsigned int t_HbmChannels,
          unsigned int t_MaxRows,
          unsigned int t_MaxCols,
          unsigned int t_HbmMemBits>
class PCG {
   public:
    PCG(){};
    CooMat allocMat(unsigned int p_m, unsigned int p_n, unsigned int p_nnz) {
        return m_spmPar.allocMat(p_m, p_n, p_nnz);
    }
    void allocVec(unsigned int p_dim) { return m_genCgVec.allocVec(p_dim); }
    CgInputVec getInputVec() {return m_genCgVec.getInputVec();}
    void partitionMat() { m_matPar =  m_spmPar.partitionMat(); }
    void initVec() {m_genCgVec.init();}
    void initDev(int p_devId, string& p_xclbinName) { 
        m_host.init(p_devId, p_xclbinName);}
    void setDat() {
        CgVector l_cgVec = m_genCgVec.getVec();
        m_host.sendDat(m_matPar.m_nnzValPtr, m_matPar.m_nnzValSize,
                       m_matPar.m_parParamPtr, m_matPar.m_parParamSize,
                       l_cgVec.h_pk, l_cgVec.vecBytes,
                       m_matPar.m_rbParamPtr, m_matPar.m_rbParamSize,
                       l_cgVec.h_Apk, l_cgVec.vecBytes,
                       l_cgVec.h_zk, l_cgVec.vecBytes,
                       l_cgVec.h_rk, l_cgVec.vecBytes,
                       l_cgVec.h_jacobi, l_cgVec.vecBytes,
                       l_cgVec.h_xk, l_cgVec.vecBytes);
    }
    void setInstr(unsigned int p_maxIter, t_DataType p_tol) {
        t_DataType l_dot = m_genCgVec.getDot();
        t_DataType l_rz = m_genCgVec.getRz();
        m_genInstr.setInstr(p_maxIter, m_genCgVec.getDimAligned(), l_dot, p_tol, l_rz);
        CgInstr l_cgInstr = m_genInstr.getInstrPtr();
        m_host.sendInstr(l_cgInstr.h_instr, l_cgInstr.h_instrBytes);
    }
    void run() {m_host.run();}
    CgVector getRes(){
        m_host.getDat();
        m_host.finish();
        return m_genCgVec.getVec();
    }
    CgInstr getInstr(){
        CgInstr l_cgInstr = m_genInstr.getInstrPtr();
        return l_cgInstr;
    }
    vector<uint32_t> getMatInfo() {
        vector<uint32_t> l_info(6);
        l_info[0] = m_matPar.m_m;
        l_info[1] = m_matPar.m_n;
        l_info[2] = m_matPar.m_nnz;
        l_info[3] = m_matPar.m_mPad;
        l_info[4] = m_matPar.m_nPad;
        l_info[5] = m_matPar.m_nnzPad; 
        return l_info;
    }

   private:
    SpmPar<t_ParEntries, t_AccLatency, t_HbmChannels, t_MaxRows, t_MaxCols, t_HbmMemBits> m_spmPar;
    GenCgVector<t_DataType, t_ParEntries> m_genCgVec;
    GenCgInstr<t_DataType, t_ParEntries, t_InstrBytes> m_genInstr;
    xCgHost m_host;
    MatPartition m_matPar;
};

#endif

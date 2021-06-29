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
#ifndef GEN_SIGNATURE_HPP_
#define GEN_SIGNATURE_HPP_

#include <string>
#include "matrix_params.hpp"
#include "signature.hpp"

CooMatInfo loadMatInfo(std::string path);
void loadMat(std::string path, CooMatInfo& p_matInfo, CooMat& p_mat);
void storeMatPar(std::string path, MatPartition& p_matPar);

template <unsigned int t_ParEntries,
          unsigned int t_AccLatency,
          unsigned int t_HbmChannels,
          unsigned int t_MaxRows,
          unsigned int t_MaxCols,
          unsigned int t_HbmMemBits>
class SpmPar {
   public:
    SpmPar() { m_sig.init(t_ParEntries, t_AccLatency, t_HbmChannels, t_MaxRows, t_MaxCols, t_HbmMemBits); }

    CooMat allocMat(uint32_t p_m, uint32_t p_n, uint32_t p_nnz) {
        CooMat l_mat;
        m_spm.init(p_m, p_n, p_nnz);
        l_mat.m_rowIdxPtr = (void*)(m_spm.m_row_list.data());
        l_mat.m_colIdxPtr = (void*)(m_spm.m_col_list.data());
        l_mat.m_datPtr = (void*)(m_spm.m_data_list.data());
        return l_mat;
    }

    MatPartition partitionMat() {
        m_spm.updateMinIdx();
        MatPartition l_res = m_sig.gen_sig(m_spm);
        return l_res;
    }

   private:
    SparseMatrix m_spm;
    Signature m_sig;
};

#endif

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

namespace xf {
namespace sparse {

CooMatInfo loadMatInfo(std::string path);
void storeMatPar(std::string path, MatPartition& p_matPar);

template <typename t_DataType>
class SpmPar {
   public:
    SpmPar(unsigned int t_ParEntries,
           unsigned int t_AccLatency,
           unsigned int t_HbmChannels,
           unsigned int t_MaxRows,
           unsigned int t_MaxCols,
           unsigned int t_HbmMemBits) {
        m_sig.init(t_ParEntries, t_AccLatency, t_HbmChannels, t_MaxRows, t_MaxCols, t_HbmMemBits);
    }

    MatPartition partitionCooMat(
        const uint32_t p_m, const uint32_t p_n, const uint32_t p_nnz, const uint32_t* p_rowIdx, const uint32_t* p_colIdx, const t_DataType* p_data) {
        SparseMatrix l_spm;
        l_spm.loadCoo(p_m, p_n, p_nnz, p_rowIdx, p_colIdx);
        MatPartition l_res = m_sig.gen_sig(l_spm, p_data);
        return l_res;
    }
    template <typename t_IdxType>
    MatPartition partitionCscSymMat(
        const uint32_t p_dim, const uint32_t p_nnz, const t_IdxType* p_rowIdx, const t_IdxType* p_colPtr, const t_DataType* p_data) {
        std::vector<t_DataType> l_cooDat = this->getCooDatFromCscSym(p_dim, p_nnz, p_rowIdx, p_colPtr, p_data);
        SparseMatrix l_spm;
        l_spm.loadCscSym(p_dim, p_nnz, p_rowIdx, p_colPtr);
        MatPartition l_res = m_sig.gen_sig(l_spm, l_cooDat.data());
        return l_res;
    }
    int checkUpdateDim(uint32_t p_m, uint32_t p_n, uint32_t p_nnz) {
        return m_sig.checkUpdateDim(p_m, p_n, p_nnz);
    }
    MatPartition updateMat(const t_DataType* p_data) {
        MatPartition l_res = m_sig.update_sig(p_data);
        return l_res;
    }
    template <typename t_IdxType>
    MatPartition updateCscSymMat(const uint32_t p_dim, const uint32_t p_nnz, const t_IdxType* p_rowIdx, const t_IdxType* p_colPtr, const t_DataType* p_data) {
        std::vector<t_DataType> l_cooDat = this->getCooDatFromCscSym(p_dim, p_nnz, p_rowIdx, p_colPtr, p_data);
        MatPartition l_res = m_sig.update_sig(l_cooDat.data());
        return l_res;
    }

   private:
    template <typename t_IdxType>
    std::vector<t_DataType> getCooDatFromCscSym(const uint32_t p_dim, const uint32_t p_nnz, const t_IdxType* p_rowIdx, const t_IdxType* p_colPtr, const t_DataType* p_data) {
        std::vector<t_DataType> l_cooDat;
        l_cooDat.resize(p_nnz);
        uint32_t l_index = 0;
        for (uint32_t j = 0; j < p_dim; j++) {
            for (t_IdxType k = p_colPtr[j] - 1; k < p_colPtr[j + 1] - 1; k++) {
                t_IdxType i = p_rowIdx[k] - 1;
                if (l_index >= p_nnz) {
                    throw SpmInternalError("faile to convert cscSym mat to coo mat");
                }
                assert(l_index < p_nnz);
                l_cooDat[l_index++] = p_data[k];
                if (i != j) {
                    if (l_index >= p_nnz) {
                        throw SpmInternalError("faile to convert cscSym mat to coo mat");
                    }
                    assert(l_index < p_nnz);
                    l_cooDat[l_index++] = p_data[k];
                }
            }
        }
        if (l_index != p_nnz) {
            throw SpmInternalError("faile to convert cscSym mat to coo mat");
        }
        assert(l_index == p_nnz);
        return l_cooDat;
    }
    Signature m_sig;
};

}
}
#endif

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
#ifndef SIGNATURE_HPP_
#define SIGNATURE_HPP_
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <assert.h>
#include <thread>
#include <unordered_map>
#include "utils.hpp"

#define DIV_CEIL(x, y) (((x) + (y)-1) / (y))
#define ZERO_VAL std::numeric_limits<uint32_t>::max()

class Signature {
   public:
    Signature() = default;
    Signature(uint32_t parEntries,
              uint32_t accLatency,
              uint32_t channels,
              uint32_t maxRows,
              uint32_t maxCols,
              uint32_t memBits) {
        m_parEntries = parEntries;
        m_accLatency = accLatency;
        m_channels = channels;
        m_maxRows = maxRows;
        m_maxCols = maxCols;
        m_memBits = memBits;
        m_rbParam.init(m_memBits, m_channels);
        m_parParam.init(m_memBits, m_channels);
        m_nnzStore.init(m_memBits, parEntries, accLatency, m_channels);
        m_chParSpms.resize(m_channels);
    }
    void init(uint32_t parEntries,
              uint32_t accLatency,
              uint32_t channels,
              uint32_t maxRows,
              uint32_t maxCols,
              uint32_t memBits) {
        m_parEntries = parEntries;
        m_accLatency = accLatency;
        m_channels = channels;
        m_maxRows = maxRows;
        m_maxCols = maxCols;
        m_memBits = memBits;
        m_rbParam.init(m_memBits, m_channels);
        m_parParam.init(m_memBits, m_channels);
        m_nnzStore.init(m_memBits, parEntries, accLatency, m_channels);
        m_chParSpms.resize(m_channels);
    }

    SparseMatrix add_spm(std::vector<uint32_t>& p_row,
                         std::vector<uint32_t>& p_col,
                         std::vector<uint32_t>& p_data,
                         uint32_t& p_m,
                         uint32_t& p_n,
                         uint32_t& p_nnz,
                         uint32_t& p_minRowId,
                         uint32_t& p_minColId) {
        SparseMatrix l_spm;
        l_spm.create_matrix(p_row, p_col, p_data);
        p_m = l_spm.getM();
        p_n = l_spm.getN();
        p_nnz = l_spm.getNnz();
        p_minRowId = l_spm.getMinRowId();
        p_minColId = l_spm.getMinColId();
        return l_spm;
    }

    void gen_rbs(SparseMatrix& p_spm, std::vector<SparseMatrix>& p_rbSpms) {
        m_mPad = p_spm.getM();
        m_nPad = DIV_CEIL(p_spm.getN(), m_parEntries) * m_parEntries;
        m_rbParam.add_dummyInfo();
        m_rbParam.m_totalRows = p_spm.getM();
        m_rbParam.m_totalRbs = 0;
        uint32_t l_numPars = 0;
        uint32_t l_sId = 0;
        uint32_t l_eId = 0;
        p_spm.sort_by_row();
        uint32_t l_minRowId = p_spm.getMinRowId();

        const std::vector<uint32_t> &l_tmp = p_spm.getRows();
        auto l_up = l_tmp.begin();
        while (l_eId < p_spm.getNnz()) {
            l_up = upper_bound(l_up, l_tmp.end(), l_minRowId + m_maxRows, isLessEqual);
            l_eId = l_up - l_tmp.begin();
            if (l_eId > l_sId) {
                std::vector<uint32_t> l_row = p_spm.getSubRows(l_sId, l_eId);
                std::vector<uint32_t> l_col = p_spm.getSubCols(l_sId, l_eId);
                std::vector<uint32_t> l_data = p_spm.getSubDatas(l_sId, l_eId);

                uint32_t l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId = 0;
                SparseMatrix l_rbSpm = add_spm(l_row, l_col, l_data, l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId);
                p_rbSpms.push_back(l_rbSpm);
                assert(l_m <= m_maxRows);
                l_sId = l_eId;
                m_rbParam.add_rbIdxInfo(l_spmMinRowId, l_spmMinColId, l_n, l_numPars);
                m_rbParam.add_rbSizeInfo(l_m, l_nnz);
                for (uint32_t i = 0; i < 6; i++) {
                    m_rbParam.add_dummyInfo();
                }
                m_rbParam.m_totalRbs += 1;
                if (l_eId < p_spm.getNnz()) {
                    l_minRowId = p_spm.getRow(l_eId);
                }
            }
        }
    }

    void genPars4Rb(unsigned int p_rbId, SparseMatrix& p_rbSpm, std::vector<SparseMatrix>& p_parSpms) {
        std::vector<uint32_t> l_rbInfo = m_rbParam.get_rbInfo(p_rbId, 1);
        assert(p_rbSpm.getNnz() == l_rbInfo[1]);
        assert(p_rbSpm.getM() == l_rbInfo[0]);
        assert(p_rbSpm.getM() <= m_maxRows);
        uint32_t l_rbPars = 0;
        p_rbSpm.sort_by_col();
        uint32_t l_minColId = (p_rbSpm.getMinColId() / m_parEntries) * m_parEntries;
        uint32_t l_sId = 0, l_eId = 0;
        const std::vector<uint32_t>& l_tmp = p_rbSpm.getCols();
        auto l_up = l_tmp.begin();
        while (l_eId < p_rbSpm.getNnz()) {
            l_up = upper_bound(l_up, l_tmp.end(), l_minColId + m_maxCols, isLessEqual);
            l_eId = l_up - l_tmp.begin();
            if (l_eId > l_sId) {
                std::vector<uint32_t> l_row = p_rbSpm.getSubRows(l_sId, l_eId);
                std::vector<uint32_t> l_col = p_rbSpm.getSubCols(l_sId, l_eId);
                std::vector<uint32_t> l_data = p_rbSpm.getSubDatas(l_sId, l_eId);
                uint32_t l_m = 0, l_n = 0, l_nnz = 0, l_spmMinRowId = 0, l_spmMinColId = 0;
                SparseMatrix l_parSpm = add_spm(l_row, l_col, l_data, l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId);
                p_parSpms.push_back(l_parSpm);
                assert(l_m <= m_maxRows);
                assert(l_n <= m_maxCols);
                l_sId = l_eId;
                l_rbPars += 1;
                if (l_eId < p_rbSpm.getNnz()) {
                    l_minColId = (p_rbSpm.getCol(l_eId) / m_parEntries) * m_parEntries;
                }
            }
        }
        m_rbParam.set_numPars(p_rbId, l_rbPars);
    }
    void gen_pars(std::vector<SparseMatrix>& p_rbSpms, std::vector<SparseMatrix>& p_parSpms) {
        uint32_t l_totalRbs = p_rbSpms.size();
        std::vector<std::vector<SparseMatrix> > l_parSpms(l_totalRbs);
#ifdef MULTITHREADS
        std::vector<std::thread> l_threads(l_totalRbs);
        for (uint32_t i = 0; i < l_totalRbs; ++i) {
            l_threads[i] = std::thread(&Signature::genPars4Rb, this, i, std::ref(p_rbSpms[i]), std::ref(l_parSpms[i]));
        }
        for (auto& th : l_threads) {
            th.join();
        }
#else
#pragma omp parallel for
        for (uint32_t i = 0; i < l_totalRbs; ++i) {
            genPars4Rb(i, p_rbSpms[i], l_parSpms[i]);
        }
#endif
        for (uint32_t i = 0; i < l_totalRbs; ++i) {
            p_parSpms.insert(p_parSpms.end(), l_parSpms[i].begin(), l_parSpms[i].end());
        }
    }

    void pad_par(SparseMatrix& p_parSpm, SparseMatrix& p_paddedParSpm) {
        uint32_t l_nnzs = p_parSpm.getNnz();

        std::vector<uint32_t> l_row;
        std::vector<uint32_t> l_col;
        std::vector<uint32_t> l_data;
        std::unordered_map<int, int> l_rowNnzs;
        for (uint32_t i = 0; i < l_nnzs; i++) {
            uint32_t r = p_parSpm.getRow(i);
            ++l_rowNnzs[r];
        }
        uint32_t l_sId = 0;
        while (l_nnzs > 0) {
            uint32_t l_rowId = p_parSpm.getRow(l_sId);
            uint32_t l_cRowNnzs = l_rowNnzs[l_rowId];
            l_nnzs -= l_cRowNnzs;
            uint32_t l_modId = 0;
            uint32_t l_idx = 0;
            uint32_t l_colIdBase = (p_parSpm.getCol(l_sId) / m_parEntries) * m_parEntries;
            uint32_t l_rRowNnzs = 0;
            while (l_idx < l_cRowNnzs) {
                uint32_t l_dataItem = p_parSpm.getData(l_sId + l_idx);
                uint32_t l_colId = p_parSpm.getCol(l_sId + l_idx);
                if (l_modId == 0) {
                    l_colIdBase = (l_colId / m_parEntries) * m_parEntries;
                }
                l_row.push_back(l_rowId);
                if (l_colId != (l_colIdBase + l_modId)) {
                    l_col.push_back(l_colIdBase + l_modId);
                    l_data.push_back(ZERO_VAL);
                } else {
                    l_col.push_back(l_colId);
                    l_data.push_back(l_dataItem);
                    l_idx += 1;
                }
                l_rRowNnzs += 1;
                l_modId = (l_modId + 1) % m_parEntries;
            }
            l_sId += l_cRowNnzs;

            while ((l_rRowNnzs % (m_parEntries * m_accLatency)) != 0) {
                l_row.push_back(l_rowId);
                l_col.push_back(l_colIdBase + l_modId);
                l_data.push_back(ZERO_VAL);
                l_modId = (l_modId + 1) % m_parEntries;
                l_rRowNnzs += 1;
            }
        }
        p_paddedParSpm.create_matrix(l_row, l_col, l_data);
    }

    void create_padPar(unsigned int p_id, SparseMatrix& p_parSpm, SparseMatrix& p_paddedParSpm) {
        p_parSpm.sort_by_row();
        pad_par(p_parSpm, p_paddedParSpm);
    }

    void gen_paddedPars(std::vector<SparseMatrix>& p_rbSpms, std::vector<SparseMatrix>& p_paddedParSpms) {
        std::vector<SparseMatrix> l_parSpms;
        gen_pars(p_rbSpms, l_parSpms);

        unsigned int l_size = l_parSpms.size();
        p_paddedParSpms.resize(l_size);

#ifdef MULTITHREADS
        std::vector<std::thread> l_threads(l_size);
        for (unsigned int i = 0; i < l_size; i++) {
            /*SparseMatrix l_parSpm = l_parSpms[i];
              l_parSpm.complete_sort_by_row();
              SparseMatrix l_paddedParSpm = pad_par(l_parSpm);
              assert(l_paddedParSpm.getM() <= m_maxRows);
              assert(l_paddedParSpm.getN() <= m_maxCols);
              p_paddedParSpms[i] = l_paddedParSpm;*/
            l_threads[i] =
                std::thread(&Signature::create_padPar, this, i, std::ref(l_parSpms[i]), std::ref(p_paddedParSpms[i]));
        }
        for (auto& th : l_threads) {
            th.join();
        }
#else
#pragma omp parallel for
        for (unsigned int i = 0; i < l_size; i++) {
            create_padPar(i, l_parSpms[i], p_paddedParSpms[i]);
        }
#endif
    }

    void gen_chPars(std::vector<SparseMatrix>& p_paddedParSpms, std::vector<std::vector<SparseMatrix> >& p_chParSpms) {
        m_parParam.add_dummyInfo();
        uint32_t l_totalPars = p_paddedParSpms.size();
        for (uint32_t i = 0; i < l_totalPars; i++) {
            SparseMatrix l_parSpm = p_paddedParSpms[i];
            assert(l_parSpm.getMinColId() % m_parEntries == 0);
            uint32_t l_baseParAddr = l_parSpm.getMinColId() / m_parEntries;
            uint32_t l_colBks = DIV_CEIL(l_parSpm.getN(), m_parEntries);
            uint32_t l_rows = l_parSpm.getM();
            uint32_t l_nnzs = l_parSpm.getNnz();
            uint32_t l_nnzsPerCh = l_nnzs / m_channels;
            m_nnzPad += l_nnzs;
            uint32_t l_sId = 0, l_eId = 0;
            uint32_t l_chBaseAddr[m_channels];
            memset(l_chBaseAddr, 0, m_channels * sizeof(uint32_t));
            uint32_t l_chCols[m_channels];
            memset(l_chCols, 0, m_channels * sizeof(uint32_t));
            uint32_t l_chNnzs[m_channels];
            memset(l_chNnzs, 0, m_channels * sizeof(uint32_t));
            for (uint32_t c = 0; c < m_channels; c++) {
                if ((c == m_channels - 1) || (l_sId + l_nnzsPerCh >= l_parSpm.getNnz())) {
                    l_eId = l_parSpm.getNnz();
                } else {
                    l_eId = l_sId + l_nnzsPerCh;
                }
                while ((l_eId > 0) && (l_eId < l_parSpm.getNnz()) &&
                        (l_parSpm.getRow(l_eId) == l_parSpm.getRow(l_eId - 1))) {
                    l_eId += 1;
                }

                std::vector<uint32_t> l_row = l_parSpm.getSubRows(l_sId, l_eId);
                std::vector<uint32_t> l_col = l_parSpm.getSubCols(l_sId, l_eId);
                std::vector<uint32_t> l_data = l_parSpm.getSubDatas(l_sId, l_eId);

                l_sId = l_eId;
                uint32_t l_m = 0, l_n = 0, l_nnz = 0, l_minRowId = 0, l_minColId = 0;
                SparseMatrix l_chParSpm = add_spm(l_row, l_col, l_data, l_m, l_n, l_nnz, l_minRowId, l_minColId);
                p_chParSpms[c].push_back(l_chParSpm);
                assert(l_m <= m_maxRows);
                assert(l_n <= m_maxCols);
                assert(l_minColId % m_parEntries == 0);
                assert(l_n % m_parEntries == 0);
                assert(l_nnz % (m_parEntries * m_accLatency) == 0);
                l_chBaseAddr[c] = (l_minColId / m_parEntries) - l_baseParAddr;
                l_chCols[c] = l_n;
                l_chNnzs[c] = l_nnz;
            }
            // assert for sum
            m_parParam.add_chInfo32(l_chCols);
            m_parParam.add_chInfo32(l_chNnzs);
            m_parParam.add_parInfo(l_baseParAddr, l_colBks, l_rows, l_nnzs);
            m_parParam.add_chInfo16(l_chBaseAddr);
            uint32_t l_chColsParEntries[m_channels];
            memset(l_chColsParEntries, 0, m_channels * sizeof(uint32_t));

            for (uint32_t c = 0; c < m_channels; c++) {
                l_chColsParEntries[c] = l_chCols[c] / m_parEntries;
            }
            m_parParam.add_chInfo16(l_chColsParEntries);
            m_parParam.add_dummyInfo();
        }
        m_parParam.m_totalPars = l_totalPars;
    }

    void update_rbParams(std::vector<std::vector<SparseMatrix> >& p_chParSpms) {
        uint32_t l_totalRbs = m_rbParam.m_totalRbs;
        uint32_t l_sRbParId = 0;
        for (uint32_t rbId = 0; rbId < l_totalRbs; rbId++) {
            std::vector<uint32_t> l_rbInfo = m_rbParam.get_rbInfo(rbId, 0);
            uint32_t l_sRbRowId = l_rbInfo[0];
            uint32_t l_rbNumPars = l_rbInfo[3];
            uint32_t l_chRbMinRowId[m_channels];
            memset(l_chRbMinRowId, 0, m_channels * sizeof(uint32_t));
            uint32_t l_chRbRows[m_channels];
            memset(l_chRbRows, 0, m_channels * sizeof(uint32_t));
            uint32_t l_chRbNnzs[m_channels];
            memset(l_chRbNnzs, 0, m_channels * sizeof(uint32_t));
            for (uint32_t c = 0; c < m_channels; c++) {
                uint32_t l_minRowId = l_sRbRowId;
                uint32_t l_endRowId = l_sRbRowId;
                for (uint32_t parId = 0; parId < l_rbNumPars; parId++) {
                    uint32_t l_parID = l_sRbParId + parId;
                    SparseMatrix l_chParSpm = p_chParSpms[c][l_parID];
                    if ((l_minRowId > l_chParSpm.getMinRowId()) && (l_chParSpm.getNnz() != 0)) {
                        l_minRowId = l_chParSpm.getMinRowId();
                    }
                    if (l_endRowId < l_chParSpm.getMinRowId() + l_chParSpm.getM()) {
                        l_endRowId = l_chParSpm.getMinRowId() + l_chParSpm.getM();
                    }
                    l_chRbNnzs[c] = l_chRbNnzs[c] + l_chParSpm.getNnz();
                }
                l_chRbMinRowId[c] = l_minRowId - l_sRbRowId;
                l_chRbRows[c] = l_endRowId - l_minRowId;
                assert(l_chRbRows[c] <= m_maxRows);
            }
            uint32_t l_sumChRbNnzs = 0;
            for (uint32_t i = 0; i < m_channels; i++) {
                l_sumChRbNnzs += l_chRbNnzs[i];
            }
            m_rbParam.set_numNnzs(rbId, l_sumChRbNnzs);
            m_rbParam.set_chInfo16(rbId, 0, l_chRbMinRowId);
            m_rbParam.set_chInfo16(rbId, 1, l_chRbRows);
            m_rbParam.set_chInfo32(rbId, l_chRbNnzs);
            l_sRbParId += l_rbNumPars;
        }
    }

    void gen_nnzStore(double* p_data) {
        m_nnzStore.reserveMem(m_nnzPad);
        for (uint32_t c = 0; c < m_channels; c++) {
            m_nnzStore.add_dummyInfo(c);
        }
        uint32_t l_memIdxWidth = m_memBits / 16;
        uint32_t l_rowIdxGap = m_parEntries * m_accLatency;
        uint32_t l_rowIdxMod = l_memIdxWidth * l_rowIdxGap;
        uint32_t l_colIdxMod = l_memIdxWidth * m_parEntries;

#pragma omp parallel for
        for (uint32_t c = 0; c < m_channels; c++) {
            uint32_t l_sParId = 0;
            for (uint32_t rbId = 0; rbId < m_rbParam.m_totalRbs; rbId++) {
                std::vector<uint32_t> l_rbInfo = m_rbParam.get_rbInfo(rbId, 0);
                uint32_t l_pars = l_rbInfo[3];
                uint32_t l_sRbRowId = l_rbInfo[0];
                for (uint32_t parId = 0; parId < l_pars; parId++) {
                    uint32_t l_parId = l_sParId + parId;
                    uint32_t l_sParColId = m_parParam.get_parInfo(l_parId)[0];

                    SparseMatrix l_chParSpm = m_chParSpms[c][l_parId];
                    m_nnzStore.m_totalRowIdxBks[c] += DIV_CEIL(l_chParSpm.getNnz(), l_rowIdxMod);
                    m_nnzStore.m_totalColIdxBks[c] += DIV_CEIL(l_chParSpm.getNnz(), l_colIdxMod);
                    m_nnzStore.m_totalNnzBks[c] += l_chParSpm.getNnz() / m_parEntries;
                    uint32_t l_sChRbRowId = m_rbParam.get_chInfo16(rbId, 0)[c] + l_sRbRowId;
                    uint32_t l_sChParColId = m_parParam.get_chInfo16(l_parId, 0)[c] + l_sParColId;
                    uint32_t l_rowIdx[l_memIdxWidth];
                    memset(l_rowIdx, 0, l_memIdxWidth * sizeof(uint32_t));
                    uint32_t l_colIdx[l_memIdxWidth];
                    memset(l_colIdx, 0, l_memIdxWidth * sizeof(uint32_t));
                    double l_nnz[m_parEntries];
                    memset(l_nnz, 0, m_parEntries * sizeof(double));
                    for (uint32_t i = 0; i < l_chParSpm.getNnz(); i = i + m_parEntries) {
                        if (i % l_rowIdxMod == 0) {
                            for (uint32_t j = 0; j < l_memIdxWidth; j++) {
                                if (i + j * l_rowIdxGap < l_chParSpm.getNnz()) {
                                    l_rowIdx[j] = l_chParSpm.getRow(i + j * l_rowIdxGap) - l_sChRbRowId;
                                }
                            }
                            m_nnzStore.add_idxArr(c, l_rowIdx);
                        }
                        if (i % l_colIdxMod == 0) {
                            for (uint32_t j = 0; j < l_memIdxWidth; j++) {
                                if (i + j * m_parEntries < l_chParSpm.getNnz()) {
                                    l_colIdx[j] =
                                        l_chParSpm.getCol(i + j * m_parEntries) / m_parEntries - l_sChParColId;
                                }
                            }
                            m_nnzStore.add_idxArr(c, l_colIdx);
                        }
                        for (uint32_t j = 0; j < m_parEntries; j++) {
                            uint32_t l_nnzIdx = l_chParSpm.getData(i + j);
                            l_nnz[j] = (l_nnzIdx == ZERO_VAL) ? 0 : p_data[l_chParSpm.getData(i + j)];
                        }
                        m_nnzStore.add_nnzArr(c, l_nnz);
                    }
                }
                l_sParId += l_pars;
            }
        }
        for (uint32_t c = 0; c < m_channels; c++) {
            m_nnzStore.m_totalBks[c] =
                m_nnzStore.m_totalRowIdxBks[c] + m_nnzStore.m_totalColIdxBks[c] + m_nnzStore.m_totalNnzBks[c];
        }
    }
    void update_nnzStore(double* p_data) {
        std::vector<uint32_t> l_bufBytes(m_channels); 
        for (uint32_t c = 0; c < m_channels; c++) {
            l_bufBytes[c]=m_memBits/8;
        }
        uint32_t l_memIdxWidth = m_memBits / 16;
        uint32_t l_rowIdxGap = m_parEntries * m_accLatency;
        uint32_t l_rowIdxMod = l_memIdxWidth * l_rowIdxGap;
        uint32_t l_colIdxMod = l_memIdxWidth * m_parEntries;

        for (uint32_t c = 0; c < m_channels; c++) {
            uint32_t l_sParId = 0;
            for (uint32_t rbId = 0; rbId < m_rbParam.m_totalRbs; rbId++) {
                std::vector<uint32_t> l_rbInfo = m_rbParam.get_rbInfo(rbId, 0);
                uint32_t l_pars = l_rbInfo[3];
                uint32_t l_sRbRowId = l_rbInfo[0];
                for (uint32_t parId = 0; parId < l_pars; parId++) {
                    uint32_t l_parId = l_sParId + parId;
                    uint32_t l_sParColId = m_parParam.get_parInfo(l_parId)[0];
                    SparseMatrix l_chParSpm = m_chParSpms[c][l_parId];
                    double l_nnz[m_parEntries];
                    memset(l_nnz, 0, m_parEntries * sizeof(double));
                    for (uint32_t i = 0; i < l_chParSpm.getNnz(); i = i + m_parEntries) {
                        if (i % l_rowIdxMod == 0) {
                            l_bufBytes[c] += 32;
                        }
                        if (i % l_colIdxMod == 0) {
                            l_bufBytes[c] += 32;
                        }
                        for (uint32_t j = 0; j < m_parEntries; j++) {
                            uint32_t l_nnzIdx = l_chParSpm.getData(i + j);
                            l_nnz[j] = (l_nnzIdx == ZERO_VAL) ? 0 : p_data[l_chParSpm.getData(i + j)];
                        }
                        m_nnzStore.update_nnzArr(c, l_bufBytes[c], l_nnz);
                        l_bufBytes[c] += 32;
                    }
                }
                l_sParId += l_pars;
            }
        }
    }

    MatPartition gen_sig(SparseMatrix& p_spm, double* p_data) {
        m_rbParam.m_buf.clear();
        m_parParam.m_buf.clear();
        for (unsigned int i=0; i<m_nnzStore.m_buf.size(); ++i) {
            m_nnzStore.m_buf[i].clear();
            m_nnzStore.m_totalBks[i] = 0;
            m_nnzStore.m_totalRowIdxBks[i] = 0;
            m_nnzStore.m_totalColIdxBks[i] = 0;
            m_nnzStore.m_totalNnzBks[i] = 0;
        } 
        m_m = p_spm.m_m;
        m_n = p_spm.m_n;
        m_nnz = p_spm.m_nnz;
        m_mPad = 0;
        m_nPad = 0;
        m_nnzPad = 0;
        std::vector<SparseMatrix> l_rbSpms;
        gen_rbs(p_spm, l_rbSpms); // write into l_rbSpms
        assert(m_rbParam.m_totalRows == p_spm.getM());
        std::vector<SparseMatrix> l_paddedParSpms;
        gen_paddedPars(l_rbSpms, l_paddedParSpms); // write into l_paddedParSpms
        for (unsigned int i=0; i<l_rbSpms.size(); ++i) {
            l_rbSpms[i].clearAll();
        }
        for (unsigned int i = 0; i < m_channels; ++i) {
            m_chParSpms[i].clear();
        }
        gen_chPars(l_paddedParSpms, m_chParSpms); // write into m_chParSpms
        for (unsigned int i=0; i<l_paddedParSpms.size(); ++i) {
            l_paddedParSpms[i].clearAll();
        }
        update_rbParams(m_chParSpms);
        gen_nnzStore(p_data);
        for (unsigned int c = 0; c < m_channels; ++c) {
            for (unsigned int i=0; i<m_chParSpms[c].size(); ++i) {
                m_chParSpms[c][i].clearRowIdx();
                m_chParSpms[c][i].clearColIdx();
            }
        }
        m_rbParam.update_buf();
        m_parParam.update_buf();
        m_nnzStore.update_buf();
        MatPartition l_res;
        l_res.m_rbParamPtr = (void*)(&(m_rbParam.m_buf[0]));
        l_res.m_rbParamSize = m_rbParam.m_buf.size();

        l_res.m_parParamPtr = (void*)(&(m_parParam.m_buf[0]));
        l_res.m_parParamSize = m_parParam.m_buf.size();

        l_res.m_nnzValPtr.resize(m_nnzStore.m_buf.size());
        for (uint32_t i = 0; i < m_nnzStore.m_buf.size(); ++i) {
            l_res.m_nnzValPtr[i] = (void*)(&(m_nnzStore.m_buf[i][0]));
            l_res.m_nnzValSize.push_back(m_nnzStore.m_buf[i].size());
        }
        l_res.m_m = m_m;
        l_res.m_n = m_n;
        l_res.m_nnz = m_nnz;
        l_res.m_mPad = m_mPad;
        l_res.m_nPad = m_nPad;
        l_res.m_nnzPad = m_nnzPad;
        return l_res;
    }

    MatPartition update_sig(double* p_data) {
        update_nnzStore(p_data);
        MatPartition l_res;
        l_res.m_rbParamPtr = (void*)(&(m_rbParam.m_buf[0]));
        l_res.m_rbParamSize = m_rbParam.m_buf.size();

        l_res.m_parParamPtr = (void*)(&(m_parParam.m_buf[0]));
        l_res.m_parParamSize = m_parParam.m_buf.size();

        l_res.m_nnzValPtr.resize(m_nnzStore.m_buf.size());
        for (uint32_t i = 0; i < m_nnzStore.m_buf.size(); ++i) {
            l_res.m_nnzValPtr[i] = (void*)(&(m_nnzStore.m_buf[i][0]));
            l_res.m_nnzValSize.push_back(m_nnzStore.m_buf[i].size());
        }
        l_res.m_m = m_m;
        l_res.m_n = m_n;
        l_res.m_nnz = m_nnz;
        l_res.m_mPad = m_mPad;
        l_res.m_nPad = m_nPad;
        l_res.m_nnzPad = m_nnzPad;
        return l_res;
    }

    void store_rbParam(std::string filename) { m_rbParam.write_file(filename); }

    void store_parParam(std::string filename) { m_parParam.write_file(filename); }

    void store_nnz(std::string* filenames) { m_nnzStore.write_file(filenames); }

    void store_info(std::string filename) {
        uint32_t int32Arr[6];
        memset(int32Arr, 0, 6 * sizeof(uint32_t));
        int32Arr[0] = m_m;
        int32Arr[1] = m_n;
        int32Arr[2] = m_nnz;
        int32Arr[3] = m_mPad;
        int32Arr[4] = m_nPad;
        int32Arr[5] = m_nnzPad;
        std::ofstream outFile(filename, std::ios::binary);
        outFile.write((char*)&int32Arr[0], sizeof(uint32_t) * 6);
    }
    int checkUpdateDim(uint32_t p_m, uint32_t p_n, uint32_t p_nnz) {
        if ((p_m == m_m) && (p_n == m_n) && (p_nnz == m_nnz)) {
            return 0;
        }
        else {
            std::cout << "ERROR: update dimensions are not the same as the stored dimensions" << std::endl;
            return -1;
        }
    }

   private:
    uint32_t m_m, m_n, m_nnz = 0;
    uint32_t m_mPad, m_nPad, m_nnzPad = 0;
    uint32_t m_parEntries, m_accLatency, m_channels, m_maxRows, m_maxCols, m_memBits;
    RowBlockParam m_rbParam;
    ParParam m_parParam;
    NnzStore m_nnzStore;
    std::vector<std::vector<SparseMatrix> > m_chParSpms;
};
#endif

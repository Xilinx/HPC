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
#include <unordered_map>

using namespace std;

#define DIV_CEIL(x, y) (((x) + (y)-1) / (y))

bool isLessEqual(int x, int y) {
    return x <= y;
}

class Signature {
   public:
    Signature(int parEntries, int accLatency, int channels, int maxRows, int maxCols, int memBits) {
        m_parEntries = parEntries;
        m_accLatency = accLatency;
        m_channels = channels;
        m_maxRows = maxRows;
        m_maxCols = maxCols;
        m_memBits = memBits;
        m_rbParam.init(m_memBits, m_channels);
        m_parParam.init(m_memBits, m_channels);
        m_nnzStore.init(m_memBits, parEntries, accLatency, m_channels);
    }

    SparseMatrix add_spm(vector<int> p_row,
                         vector<int> p_col,
                         vector<double> p_data,
                         int& p_m,
                         int& p_n,
                         int& p_nnz,
                         int& p_minRowId,
                         int& p_minColId) {
        SparseMatrix l_spm;
        l_spm.create_matrix(p_row, p_col, p_data);
        p_m = l_spm.getM();
        p_n = l_spm.getN();
        p_nnz = l_spm.getNnz();
        p_minRowId = l_spm.getMinRowId();
        p_minColId = l_spm.getMinColId();
        return l_spm;
    }

    void gen_rbs(SparseMatrix& p_spm, vector<SparseMatrix>& p_rbSpms) {
        m_mPad = p_spm.getM();
        m_nPad = DIV_CEIL(p_spm.getN(), m_parEntries) * m_parEntries;
        m_rbParam.add_dummyInfo();
        m_rbParam.m_totalRows = p_spm.getM();
        m_rbParam.m_totalRbs = 0;
        int l_numPars = 0;
        int l_sId = 0;
        int l_eId = 0;
        p_spm.sort_by_row();
        int l_minRowId = p_spm.getMinRowId();

        while (l_eId < p_spm.getNnz()) {
            if (p_spm.getRow(p_spm.getNnz() - 1) < (l_minRowId + m_maxRows)) {
                l_eId = p_spm.getNnz();
            } else {
                vector<int> l_tmp = p_spm.getRows();
                auto l_up = upper_bound(l_tmp.begin(), l_tmp.end(), l_minRowId + m_maxRows, isLessEqual);
                l_eId = l_up - l_tmp.begin();
            }
            if (l_eId > l_sId) {
                vector<int> l_row = p_spm.getSubRows(l_sId, l_eId);
                vector<int> l_col = p_spm.getSubCols(l_sId, l_eId);
                vector<double> l_data = p_spm.getSubDatas(l_sId, l_eId);

                int l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId = 0;
                SparseMatrix l_rbSpm = add_spm(l_row, l_col, l_data, l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId);
                p_rbSpms.push_back(l_rbSpm);
                assert(l_m <= m_maxRows);
                l_sId = l_eId;
                m_rbParam.add_rbIdxInfo(l_spmMinRowId, l_spmMinColId, l_n, l_numPars);
                m_rbParam.add_rbSizeInfo(l_m, l_nnz);
                for (int i = 0; i < 6; i++) {
                    m_rbParam.add_dummyInfo();
                }
                m_rbParam.m_totalRbs += 1;
                if (l_eId < p_spm.getNnz()) {
                    l_minRowId = p_spm.getRow(l_eId);
                }
            }
        }
    }

    void gen_pars(vector<SparseMatrix>& p_rbSpms, vector<SparseMatrix>& p_parSpms) {
        int l_totalPars = 0;
        int l_totalRbs = p_rbSpms.size();
        for (int i = 0; i < l_totalRbs; i++) {
            SparseMatrix l_rbSpm = p_rbSpms[i];
            vector<int> l_rbInfo = m_rbParam.get_rbInfo(i, 1);

            assert(l_rbSpm.getNnz() == l_rbInfo[1]);
            assert(l_rbSpm.getM() == l_rbInfo[0]);
            assert(l_rbSpm.getM() <= m_maxRows);
            int l_rbPars = 0;
            l_rbSpm.sort_by_col();
            int l_minColId = (l_rbSpm.getMinColId() / m_parEntries) * m_parEntries;
            int l_sId = 0, l_eId = 0;
            while (l_eId < l_rbSpm.getNnz()) {
                if (l_rbSpm.getCol(l_rbSpm.getNnz() - 1) < l_minColId + m_maxCols) {
                    l_eId = l_rbSpm.getNnz();
                } else {
                    vector<int> l_tmp = l_rbSpm.getCols();
                    auto l_up = upper_bound(l_tmp.begin(), l_tmp.end(), l_minColId + m_maxCols, isLessEqual);
                    l_eId = l_up - l_tmp.begin();
                }
                if (l_eId > l_sId) {
                    vector<int> l_row = l_rbSpm.getSubRows(l_sId, l_eId);
                    vector<int> l_col = l_rbSpm.getSubCols(l_sId, l_eId);
                    vector<double> l_data = l_rbSpm.getSubDatas(l_sId, l_eId);
                    int l_m = 0, l_n = 0, l_nnz = 0, l_spmMinRowId = 0, l_spmMinColId = 0;
                    SparseMatrix l_parSpm =
                        add_spm(l_row, l_col, l_data, l_m, l_n, l_nnz, l_spmMinRowId, l_spmMinColId);
                    p_parSpms.push_back(l_parSpm);
                    assert(l_m <= m_maxRows);
                    assert(l_n <= m_maxCols);
                    l_sId = l_eId;
                    l_rbPars += 1;
                    if (l_eId < l_rbSpm.getNnz()) {
                        l_minColId = (l_rbSpm.getCol(l_eId) / m_parEntries) * m_parEntries;
                    }
                }
            }
            m_rbParam.set_numPars(i, l_rbPars);
            l_totalPars += l_rbPars;
        }
    }

    SparseMatrix pad_par(SparseMatrix p_parSpm) {
        int l_nnzs = p_parSpm.getNnz();
        int l_rows = p_parSpm.getM();
        int l_parNnzs = 0;

        vector<int> l_row;
        vector<int> l_col;
        vector<double> l_data;
        unordered_map<int, int> l_rowNnzs;
        for (int i = 0; i < l_nnzs; i++) {
            int r = p_parSpm.getRow(i);
            ++l_rowNnzs[r];
        }
        int l_sId = 0;
        while (l_nnzs > 0) {
            int l_rowId = p_parSpm.getRow(l_sId);
            int l_cRowNnzs = l_rowNnzs[l_rowId];
            l_nnzs -= l_cRowNnzs;
            int l_modId = 0;
            int l_idx = 0;
            int l_colIdBase = (p_parSpm.getCol(l_sId) / m_parEntries) * m_parEntries;
            int l_rRowNnzs = 0;
            while (l_idx < l_cRowNnzs) {
                double l_dataItem = p_parSpm.getData(l_sId + l_idx);
                int l_colId = p_parSpm.getCol(l_sId + l_idx);
                if (l_modId == 0) {
                    l_colIdBase = (l_colId / m_parEntries) * m_parEntries;
                }
                if (l_colId != (l_colIdBase + l_modId)) {
                    l_row.push_back(l_rowId);
                    l_col.push_back(l_colIdBase + l_modId);
                    l_data.push_back(0);
                } else {
                    l_row.push_back(l_rowId);
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
                l_data.push_back(0);
                l_modId = (l_modId + 1) % m_parEntries;
                l_rRowNnzs += 1;
            }
        }

        SparseMatrix l_paddedParSpm;
        l_paddedParSpm.create_matrix(l_row, l_col, l_data);

        return l_paddedParSpm;
    }

    void gen_paddedPars(vector<SparseMatrix>& p_rbSpms, vector<SparseMatrix>& p_paddedParSpms) {
        vector<SparseMatrix> l_parSpms;
        gen_pars(p_rbSpms, l_parSpms);

        for (int i = 0; i < l_parSpms.size(); i++) {
            SparseMatrix l_parSpm = l_parSpms[i];
            l_parSpm.sort_by_row();
            SparseMatrix l_paddedParSpm = pad_par(l_parSpm);
            assert(l_paddedParSpm.getM() <= m_maxRows);
            assert(l_paddedParSpm.getN() <= m_maxCols);
            p_paddedParSpms.push_back(l_paddedParSpm);
        }
    }

    void gen_chPars(vector<SparseMatrix>& p_paddedParSpms, vector<vector<SparseMatrix> >& p_chParSpms) {
        m_parParam.add_dummyInfo();
        int l_totalPars = p_paddedParSpms.size();
        for (int i = 0; i < l_totalPars; i++) {
            SparseMatrix l_parSpm = p_paddedParSpms[i];
            assert(l_parSpm.getMinColId() % m_parEntries == 0);
            int l_baseParAddr = l_parSpm.getMinColId() / m_parEntries;
            int l_colBks = DIV_CEIL(l_parSpm.getN(), m_parEntries);
            int l_rows = l_parSpm.getM();
            int l_nnzs = l_parSpm.getNnz();
            int l_nnzsPerCh = l_nnzs / m_channels;
            m_nnzPad += l_nnzs;
            int l_sId = 0, l_eId = 0;
            int l_chBaseAddr[m_channels];
            memset(l_chBaseAddr, 0, m_channels * sizeof(int));
            int l_chCols[m_channels];
            memset(l_chCols, 0, m_channels * sizeof(int));
            int l_chNnzs[m_channels];
            memset(l_chNnzs, 0, m_channels * sizeof(int));
            for (int c = 0; c < m_channels; c++) {
                if ((c == m_channels - 1) || (l_sId + l_nnzsPerCh >= l_parSpm.getNnz())) {
                    l_eId = l_parSpm.getNnz();
                } else {
                    l_eId = l_sId + l_nnzsPerCh;
                }
                while ((l_eId > 0) && (l_eId < l_parSpm.getNnz()) &&
                       (l_parSpm.getRow(l_eId) == l_parSpm.getRow(l_eId - 1))) {
                    l_eId += 1;
                }

                vector<int> l_row = l_parSpm.getSubRows(l_sId, l_eId);
                vector<int> l_col = l_parSpm.getSubCols(l_sId, l_eId);
                vector<double> l_data = l_parSpm.getSubDatas(l_sId, l_eId);

                l_sId = l_eId;
                int l_m = 0, l_n = 0, l_nnz = 0, l_minRowId = 0, l_minColId = 0;
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
            int l_chColsParEntries[m_channels];
            memset(l_chColsParEntries, 0, m_channels * sizeof(int));

            for (int c = 0; c < m_channels; c++) {
                l_chColsParEntries[c] = l_chCols[c] / m_parEntries;
            }
            m_parParam.add_chInfo16(l_chColsParEntries);
            m_parParam.add_dummyInfo();
        }
        m_parParam.m_totalPars = l_totalPars;
    }

    void update_rbParams(vector<vector<SparseMatrix> >& p_chParSpms) {
        int l_totalRbs = m_rbParam.m_totalRbs;
        int l_sRbParId = 0;
        for (int rbId = 0; rbId < l_totalRbs; rbId++) {
            vector<int> l_rbInfo = m_rbParam.get_rbInfo(rbId, 0);
            int l_sRbRowId = l_rbInfo[0];
            int l_minRbColId = l_rbInfo[1];
            int l_rbCols = l_rbInfo[2];
            int l_rbNumPars = l_rbInfo[3];
            int l_chRbMinRowId[m_channels];
            memset(l_chRbMinRowId, 0, m_channels * sizeof(int));
            int l_chRbRows[m_channels];
            memset(l_chRbRows, 0, m_channels * sizeof(int));
            int l_chRbNnzs[m_channels];
            memset(l_chRbNnzs, 0, m_channels * sizeof(int));
            for (int c = 0; c < m_channels; c++) {
                int l_minRowId = l_sRbRowId;
                int l_endRowId = l_sRbRowId;
                for (int parId = 0; parId < l_rbNumPars; parId++) {
                    int l_parID = l_sRbParId + parId;
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
            int l_sumChRbNnzs = 0;
            for (int i = 0; i < m_channels; i++) {
                l_sumChRbNnzs += l_chRbNnzs[i];
            }
            m_rbParam.set_numNnzs(rbId, l_sumChRbNnzs);
            m_rbParam.set_chInfo16(rbId, 0, l_chRbMinRowId);
            m_rbParam.set_chInfo16(rbId, 1, l_chRbRows);
            m_rbParam.set_chInfo32(rbId, l_chRbNnzs);
            l_sRbParId += l_rbNumPars;
        }
    }

    void gen_nnzStore(vector<vector<SparseMatrix> >& p_chParSpms) {
        for (int c = 0; c < m_channels; c++) {
            m_nnzStore.add_dummyInfo(c);
        }
        int l_memIdxWidth = m_memBits / 16;
        int l_rowIdxGap = m_parEntries * m_accLatency;
        int l_rowIdxMod = l_memIdxWidth * l_rowIdxGap;
        int l_colIdxMod = l_memIdxWidth * m_parEntries;
        int l_sParId = 0;

        for (int rbId = 0; rbId < m_rbParam.m_totalRbs; rbId++) {
            vector<int> l_rbInfo = m_rbParam.get_rbInfo(rbId, 0);
            int l_pars = l_rbInfo[3];
            int l_sRbRowId = l_rbInfo[0];
            for (int parId = 0; parId < l_pars; parId++) {
                int l_parId = l_sParId + parId;
                int l_sParColId = m_parParam.get_parInfo(l_parId)[0];
                for (int c = 0; c < m_channels; c++) {
                    SparseMatrix l_chParSpm = p_chParSpms[c][l_parId];
                    m_nnzStore.m_totalRowIdxBks[c] += DIV_CEIL(l_chParSpm.getNnz(), l_rowIdxMod);
                    m_nnzStore.m_totalColIdxBks[c] += DIV_CEIL(l_chParSpm.getNnz(), l_colIdxMod);
                    m_nnzStore.m_totalNnzBks[c] += l_chParSpm.getNnz() / m_parEntries;
                    int l_sChRbRowId = m_rbParam.get_chInfo16(rbId, 0)[c] + l_sRbRowId;
                    int l_sChParColId = m_parParam.get_chInfo16(l_parId, 0)[c] + l_sParColId;
                    int l_rowIdx[l_memIdxWidth];
                    memset(l_rowIdx, 0, l_memIdxWidth * sizeof(int));
                    int l_colIdx[l_memIdxWidth];
                    memset(l_colIdx, 0, l_memIdxWidth * sizeof(int));
                    double l_nnz[m_parEntries];
                    memset(l_nnz, 0, m_parEntries * sizeof(double));
                    for (int i = 0; i < l_chParSpm.getNnz(); i = i + m_parEntries) {
                        if (i % l_rowIdxMod == 0) {
                            for (int j = 0; j < l_memIdxWidth; j++) {
                                if (i + j * l_rowIdxGap < l_chParSpm.getNnz()) {
                                    l_rowIdx[j] = l_chParSpm.getRow(i + j * l_rowIdxGap) - l_sChRbRowId;
                                }
                            }
                            m_nnzStore.add_idxArr(c, l_rowIdx);
                        }
                        if (i % l_colIdxMod == 0) {
                            for (int j = 0; j < l_memIdxWidth; j++) {
                                if (i + j * m_parEntries < l_chParSpm.getNnz()) {
                                    l_colIdx[j] =
                                        l_chParSpm.getCol(i + j * m_parEntries) / m_parEntries - l_sChParColId;
                                }
                            }
                            m_nnzStore.add_idxArr(c, l_colIdx);
                        }
                        for (int j = 0; j < m_parEntries; j++) {
                            l_nnz[j] = l_chParSpm.getData(i + j);
                        }
                        m_nnzStore.add_nnzArr(c, l_nnz);
                    }
                }
            }
            l_sParId += l_pars;
        }
        for (int c = 0; c < m_channels; c++) {
            m_nnzStore.m_totalBks[c] =
                m_nnzStore.m_totalRowIdxBks[c] + m_nnzStore.m_totalColIdxBks[c] + m_nnzStore.m_totalNnzBks[c];
        }
    }

    void process(string path) {
        ifstream l_info(path + "infos.txt");
        vector<string> infos;
        for (string line; getline(l_info, line);) {
            infos.push_back(line);
        }
        string matrix_name = infos[0];

        m_m = stoi(infos[1]);
        m_n = stoi(infos[2]);
        m_nnz = stoi(infos[3]);

        SparseMatrix l_spm = SparseMatrix(m_m, m_n, m_nnz);
        l_spm.load_row(path + "row.bin");
        l_spm.load_col(path + "col.bin");
        l_spm.load_data(path + "data.bin");

        vector<SparseMatrix> l_rbSpms;
        gen_rbs(l_spm, l_rbSpms); // write into l_rbSpms
        assert(m_rbParam.m_totalRows == l_spm.getM());
        vector<SparseMatrix> l_paddedParSpms;
        gen_paddedPars(l_rbSpms, l_paddedParSpms); // write into l_paddedParSpms
        vector<vector<SparseMatrix> > l_chParSpms;
        l_chParSpms.resize(m_channels);
        gen_chPars(l_paddedParSpms, l_chParSpms); // write into l_chParSpms
        update_rbParams(l_chParSpms);
        gen_nnzStore(l_chParSpms);
        printf("INFO: matrix %s partiton done.\n", matrix_name.c_str());
        printf("      Original m, n, nnzs = %d, %d, %d\n", m_m, m_n, m_nnz);
        printf("      After padding m, n, nnzs = %d, %d, %d\n", m_mPad, m_nPad, m_nnzPad);
        printf("      Padding overhead is %f\n", (double)(m_nnzPad - m_nnz) / m_nnz);
    }
    
    MatPartition gen_sig(const CooMat& p_mat) {
        m_m = p_mat.m_m;
        m_n = p_mat.m_n;
        m_nnz = p_mat.m_nnz;
        SparseMatrix l_spm = SparseMatrix(m_m, m_n, m_nnz);
        l_spm.set_row(p_mat.m_rowIdxPtr);
        l_spm.set_col(p_mat.m_colIdxPtr);
        l_spm.set_data(p_mat.m_datPtr);
        vector<SparseMatrix> l_rbSpms;
        gen_rbs(l_spm, l_rbSpms); // write into l_rbSpms
        assert(m_rbParam.m_totalRows == l_spm.getM());   
        vector<SparseMatrix> l_paddedParSpms;
        gen_paddedPars(l_rbSpms, l_paddedParSpms); // write into l_paddedParSpms
        vector<vector<SparseMatrix> > l_chParSpms;
        l_chParSpms.resize(m_channels);
        gen_chPars(l_paddedParSpms, l_chParSpms); // write into l_chParSpms
        update_rbParams(l_chParSpms);
        gen_nnzStore(l_chParSpms);
        m_rbParam.update_buf();
        m_parParam.update_buf();
        m_nnzStore.update_buf();
        MatPartition l_res;
        l_res.m_rbParamPtr = aligned_alloc(4096, m_rbParam.m_buf.size());
        memcpy(l_res.m_rbParamPtr, &(m_rbParam.m_buf[0]), m_rbParam.m_buf.size());
        l_res.m_rbParamSize = m_rbParam.m_buf.size();

        l_res.m_parParamPtr = aligned_alloc(4096, m_parParam.m_buf.size());
        memcpy(l_res.m_parParamPtr, &(m_parParam.m_buf[0]), m_parParam.m_buf.size());
        l_res.m_parParamSize = m_parParam.m_buf.size();

        l_res.m_nnzValPtr = (void**)malloc(sizeof(void*)*m_nnzStore.m_buf.size());
        l_res.m_nnzValSize = (uint32_t*)malloc(sizeof(uint32_t)*m_nnzStore.m_buf.size());
        for (int i=0; i<m_nnzStore.m_buf.size(); ++i) {
            l_res.m_nnzValPtr[i] = aligned_alloc(4096, m_nnzStore.m_buf[i].size());
            memcpy(l_res.m_nnzValPtr[i],  &(m_nnzStore.m_buf[i][0]), m_nnzStore.m_buf[i].size());
            l_res.m_nnzValSize[i] = m_nnzStore.m_buf[i].size();
        }
        l_res.m_m = m_m;
        l_res.m_n = m_n;
        l_res.m_nnz = m_nnz;
        l_res.m_mPad = m_mPad;
        l_res.m_nPad = m_nPad;
        l_res.m_nnzPad = m_nnzPad;
        return l_res;
    }

    void store_rbParam(string filename) { m_rbParam.write_file(filename); }

    void store_parParam(string filename) { m_parParam.write_file(filename); }

    void store_nnz(string* filenames) { m_nnzStore.write_file(filenames); }

    void store_info(string filename) {
        int int32Arr[6];
        memset(int32Arr, 0, 6 * sizeof(int));
        int32Arr[0] = m_m;
        int32Arr[1] = m_n;
        int32Arr[2] = m_nnz;
        int32Arr[3] = m_mPad;
        int32Arr[4] = m_nPad;
        int32Arr[5] = m_nnzPad;
        ofstream outFile(filename, ios::binary);
        outFile.write((char*)&int32Arr[0], sizeof(int) * 6);
    }

   private:
    int m_m, m_n, m_nnz = 0;
    int m_mPad, m_nPad, m_nnzPad = 0;
    int m_parEntries, m_accLatency, m_channels, m_maxRows, m_maxCols, m_memBits;
    RowBlockParam m_rbParam;
    ParParam m_parParam;
    NnzStore m_nnzStore;
};
#endif

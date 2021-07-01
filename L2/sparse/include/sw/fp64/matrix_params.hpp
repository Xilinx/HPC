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
#ifndef MATRIX_PARAMS_HPP_
#define MATRIX_PARAMS_HPP_
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <cmath>
#include <array>
#include <algorithm>
#include <numeric>
#include <vector>
#include <assert.h>
#include <thread>
#include "binFiles.hpp"
#include "utils.hpp"

class SparseMatrix {
   public:
    SparseMatrix() = default;
    SparseMatrix(uint32_t m, uint32_t n, uint32_t nnz) {
        m_m = m;
        m_n = n;
        m_nnz = nnz;
    }
    void updateMinIdx() {
        m_minRowId = *(min_element(m_row_list.begin(), m_row_list.end()));
        m_minColId = *(min_element(m_col_list.begin(), m_col_list.end()));
    }

    void loadCoo(uint32_t p_m, uint32_t p_n, uint32_t p_nnz, uint32_t* p_rowIdx, uint32_t* p_colIdx) {
        m_m = p_m;
        m_n = p_n;
        m_nnz = p_nnz;
        m_minRowId = 0;
        m_minColId = 0;
        m_row_list.insert(m_row_list.end(), p_rowIdx, p_rowIdx + p_nnz);
        m_col_list.insert(m_col_list.end(), p_colIdx, p_colIdx + p_nnz);
        m_data_list.resize(m_nnz);
        iota(m_data_list.begin(), m_data_list.end(), 0);
        m_minRowId = *(min_element(m_row_list.begin(), m_row_list.end()));
        m_minColId = *(min_element(m_col_list.begin(), m_col_list.end()));
    }

    void loadCscSym(uint32_t p_n, uint32_t p_nnz, uint32_t* p_rowIdx, uint32_t* p_colPtr) {
        m_m = p_n;
        m_n = p_n;
        m_nnz = p_nnz;
        m_minRowId = 0;
        m_minColId = 0;
        m_row_list.resize(m_nnz);
        m_col_list.resize(m_nnz);
        m_data_list.resize(m_nnz);
        iota(m_data_list.begin(), m_data_list.end(), 0);

        uint32_t index = 0;
        for (uint32_t j = 0; j < p_n; j++) {
            for (uint32_t k = p_colPtr[j] - 1; k < p_colPtr[j + 1] - 1; k++) {
                uint32_t i = p_rowIdx[k] - 1;
                assert(index < m_nnz);
                m_row_list[index] = i;
                m_col_list[index] = j;
                if (i != j) {
                    assert(index < m_nnz);
                    m_row_list[index] = j;
                    m_col_list[index] = i;
                }
            }
        }
        assert(index == m_nnz);
        m_minRowId = *(min_element(m_row_list.begin(), m_row_list.end()));
        m_minColId = *(min_element(m_col_list.begin(), m_col_list.end()));
    }
    void loadCscSym(uint32_t p_n, uint32_t p_nnz, int64_t* p_rowIdx, int64_t* p_colPtr) {
        m_m = p_n;
        m_n = p_n;
        m_nnz = p_nnz;
        m_minRowId = 0;
        m_minColId = 0;
        m_row_list.resize(m_nnz);
        m_col_list.resize(m_nnz);
        m_data_list.resize(m_nnz);
        iota(m_data_list.begin(), m_data_list.end(), 0);

        uint32_t index = 0;
        for (uint32_t j = 0; j < p_n; j++) {
            for (uint32_t k = p_colPtr[j] - 1; k < p_colPtr[j + 1] - 1; k++) {
                uint32_t i = p_rowIdx[k] - 1;
                assert(index < m_nnz);
                m_row_list[index] = i;
                m_col_list[index] = j;
                index++;
                if (i != j) {
                    assert(index < m_nnz);
                    m_row_list[index] = j;
                    m_col_list[index] = i;
                    index++;
                }
            }
        }
        assert(index == m_nnz);
        m_minRowId = *(min_element(m_row_list.begin(), m_row_list.end()));
        m_minColId = *(min_element(m_col_list.begin(), m_col_list.end()));
    }

    void create_matrix(std::vector<uint32_t> p_row, std::vector<uint32_t> p_col, std::vector<uint32_t> p_data) {
        if (!p_row.empty()) {
            m_nnz = p_row.size();
            m_row_list = p_row;
            m_col_list = p_col;
            m_data_list = p_data;
            m_minRowId = *(min_element(m_row_list.begin(), m_row_list.end()));
            m_minColId = *(min_element(m_col_list.begin(), m_col_list.end()));
            uint32_t l_maxRowId = *(max_element(m_row_list.begin(), m_row_list.end()));
            uint32_t l_maxColId = *(max_element(m_col_list.begin(), m_col_list.end()));
            m_m = l_maxRowId - m_minRowId + 1;
            m_n = l_maxColId - m_minColId + 1;
        } else {
            m_nnz = 0;
            m_m = 0;
            m_n = 0;
            m_minRowId = 0;
            m_minColId = 0;
        }
    }

    uint32_t getRow(uint32_t index) { return m_row_list[index]; }

    std::vector<uint32_t> getRows() { return m_row_list; }

    uint32_t getCol(uint32_t index) { return m_col_list[index]; }

    std::vector<uint32_t> getCols() { return m_col_list; }

    uint32_t getData(uint32_t index) { return m_data_list[index]; }

    std::vector<uint32_t> getDatas() { return m_data_list; }

    uint32_t getM() { return m_m; }

    uint32_t getN() { return m_n; }

    uint32_t getNnz() { return m_nnz; }

    uint32_t getMinRowId() { return m_minRowId; }
    uint32_t getMinColId() { return m_minColId; }

    std::vector<uint32_t> getSubRows(uint32_t start, uint32_t end) {
        std::vector<uint32_t> l_row(m_row_list.begin() + start, m_row_list.begin() + end);
        return l_row;
    }
    std::vector<uint32_t> getSubCols(uint32_t start, uint32_t end) {
        std::vector<uint32_t> l_col(m_col_list.begin() + start, m_col_list.begin() + end);
        return l_col;
    }
    std::vector<uint32_t> getSubDatas(uint32_t start, uint32_t end) {
        std::vector<uint32_t> l_data(m_data_list.begin() + start, m_data_list.begin() + end);
        return l_data;
    }

    void partialSort(uint32_t p_sId, unsigned int p_size, std::vector<uint32_t>& p_list, std::vector<uint32_t>& p_idx) {
        p_idx.resize(p_size);
        iota(p_idx.begin(), p_idx.end(), p_sId);
        stable_sort(p_idx.begin(), p_idx.end(), [&](uint32_t i1, uint32_t i2) { return p_list[i1] < p_list[i2]; });
    }

    void mergeIdx(unsigned int p_size,
                  std::vector<uint32_t> p_idxArr[2],
                  std::vector<uint32_t>& p_list,
                  std::vector<uint32_t>& p_idx) {
        std::vector<unsigned int> l_ids(2, 0);
        for (unsigned int i = 0; i < p_size; ++i) {
            if (l_ids[0] < p_idxArr[0].size()) {
                if ((l_ids[1] < p_idxArr[1].size()) &&
                    (p_list[p_idxArr[1][l_ids[1]]] < p_list[p_idxArr[0][l_ids[0]]])) {
                    p_idx[i] = p_idxArr[1][l_ids[1]];
                    l_ids[1]++;
                } else {
                    p_idx[i] = p_idxArr[0][l_ids[0]];
                    l_ids[0]++;
                }
            } else {
                p_idx[i] = p_idxArr[1][l_ids[1]];
                l_ids[1]++;
            }
        }
    }

    void sort_by_row() {
        std::vector<uint32_t> idx(m_nnz);
        std::vector<uint32_t> l_idxArr[2];
        std::thread t1(&SparseMatrix::partialSort, this, 0, m_nnz / 2, ref(m_row_list), ref(l_idxArr[0]));
        std::thread t2(&SparseMatrix::partialSort, this, m_nnz / 2, m_nnz - m_nnz / 2, ref(m_row_list),
                       ref(l_idxArr[1]));
        t1.join();
        t2.join();
        mergeIdx(m_nnz, l_idxArr, m_row_list, idx);
        // then sort each part using this index
        uint32_t* l_row_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_col_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_data_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        for (uint32_t i = 0; i < m_nnz; i++) {
            l_row_list[i] = m_row_list[idx[i]];
            l_col_list[i] = m_col_list[idx[i]];
            l_data_list[i] = m_data_list[idx[i]];
        }
        memcpy(&m_row_list[0], l_row_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_col_list[0], l_col_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_data_list[0], l_data_list, m_nnz * sizeof(uint32_t));
        free(l_row_list);
        free(l_col_list);
        free(l_data_list);
    }
    void complete_sort_by_row() {
        std::vector<uint32_t> idx(m_nnz);
        iota(idx.begin(), idx.end(), 0);
        stable_sort(idx.begin(), idx.end(), [this](uint32_t i1, uint32_t i2) {
            return (m_row_list[i1] < m_row_list[i2])
                       ? true
                       : (m_row_list[i1] == m_row_list[i2]) ? m_col_list[i1] <= m_col_list[i2] : false;
        });
        // then sort each part using this index
        uint32_t* l_row_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_col_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_data_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        for (uint32_t i = 0; i < m_nnz; i++) {
            l_row_list[i] = m_row_list[idx[i]];
            l_col_list[i] = m_col_list[idx[i]];
            l_data_list[i] = m_data_list[idx[i]];
        }
        memcpy(&m_row_list[0], l_row_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_col_list[0], l_col_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_data_list[0], l_data_list, m_nnz * sizeof(uint32_t));
        free(l_row_list);
        free(l_col_list);
        free(l_data_list);
    }

    void sort_by_col() {
        std::vector<uint32_t> idx(m_nnz);
        std::vector<uint32_t> l_idxArr[2];
        std::thread t1(&SparseMatrix::partialSort, this, 0, m_nnz / 2, ref(m_col_list), ref(l_idxArr[0]));
        std::thread t2(&SparseMatrix::partialSort, this, m_nnz / 2, m_nnz - m_nnz / 2, ref(m_col_list),
                       ref(l_idxArr[1]));
        t1.join();
        t2.join();
        mergeIdx(m_nnz, l_idxArr, m_col_list, idx);
        // then sort each part using this index
        uint32_t* l_row_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_col_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        uint32_t* l_data_list = (uint32_t*)malloc(m_nnz * sizeof(uint32_t));
        for (uint32_t i = 0; i < m_nnz; i++) {
            l_row_list[i] = m_row_list[idx[i]];
            l_col_list[i] = m_col_list[idx[i]];
            l_data_list[i] = m_data_list[idx[i]];
        }
        memcpy(&m_row_list[0], l_row_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_col_list[0], l_col_list, m_nnz * sizeof(uint32_t));
        memcpy(&m_data_list[0], l_data_list, m_nnz * sizeof(uint32_t));
        free(l_row_list);
        free(l_col_list);
        free(l_data_list);
    }

   public:
    uint32_t m_m, m_n, m_nnz;
    std::vector<uint32_t> m_row_list;
    std::vector<uint32_t> m_col_list;
    std::vector<uint32_t>
        m_data_list; // stores the idx of the original data array in sparse matrix, value =0 if idx == nnzs
    uint32_t m_minRowId, m_minColId;
};

class RowBlockParam {
   public:
    uint32_t m_totalRows, m_totalRbs;

    RowBlockParam() = default;

    RowBlockParam(uint32_t p_memBits, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalRows = 0;
        m_totalRbs = 0;
    }

    void init(uint32_t p_memBits, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalRows = 0;
        m_totalRbs = 0;
    }

    void add_rbIdxInfo(uint32_t p_minRowId, uint32_t p_minColId, uint32_t p_numCols, uint32_t p_numPars) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = p_minRowId;
        int32Arr[1] = p_minColId;
        int32Arr[2] = p_numCols;
        int32Arr[3] = p_numPars;
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(uint32_t));
    }

    void add_rbSizeInfo(uint32_t p_numRows, uint32_t p_numNnzs) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = p_numRows;
        int32Arr[1] = p_numNnzs;
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(uint32_t));
    }

    void add_dummyInfo() {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(uint32_t));
    }

    uint32_t get_rb_offset(uint32_t p_rbId) {
        uint32_t l_offset = m_memBytes;
        l_offset += p_rbId * m_memBytes * 8;
        return l_offset;
    }

    std::vector<uint32_t> get_rbInfo(uint32_t p_rbId, uint32_t p_rbInfoId) {
        uint32_t l_size = m_memBytes / 4;
        uint32_t l_offset = get_rb_offset(p_rbId);
        l_offset += p_rbInfoId * m_memBytes;

        std::vector<uint32_t> l_buf;
        l_buf.resize(l_size);
        memcpy(&l_buf[0], &m_buf[l_offset], l_size * sizeof(uint32_t));
        return l_buf;
    }

    void set_numPars(uint32_t p_rbId, uint32_t p_numPars) {
        uint32_t l_offset = get_rb_offset(p_rbId);
        uint32_t l_size = m_memBytes / 4;

        std::vector<uint32_t> int32Arr;
        int32Arr.resize(l_size);
        memcpy(&int32Arr[0], &m_buf[l_offset], l_size * sizeof(uint32_t));
        int32Arr[3] = p_numPars;
        memcpy(&m_buf[l_offset], &int32Arr[0], l_size * sizeof(uint32_t));
    }

    void set_numNnzs(uint32_t p_rbId, uint32_t p_numNnzs) {
        uint32_t l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes;
        uint32_t l_size = m_memBytes / 4;
        std::vector<uint32_t> int32Arr;
        int32Arr.resize(l_size);
        memcpy(&int32Arr[0], &m_buf[l_offset], l_size * sizeof(uint32_t));
        int32Arr[1] = p_numNnzs;
        memcpy(&m_buf[l_offset], &int32Arr[0], l_size * sizeof(uint32_t));
    }

    void set_chInfo16(uint32_t p_rbId, uint32_t p_chInfo16Id, uint32_t* p_info) {
        uint16_t chInfo16Arr[m_channels];
        memset(chInfo16Arr, 0, m_channels * sizeof(uint16_t));

        for (uint32_t i = 0; i < m_channels; i++) {
            chInfo16Arr[i] = p_info[i];
        }
        uint32_t l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * (2 + p_chInfo16Id);
        memcpy(&m_buf[l_offset], reinterpret_cast<uint8_t*>(chInfo16Arr), m_memBytes * sizeof(uint8_t));
    }

    void set_chInfo32(uint32_t p_rbId, uint32_t* p_info) {
        uint32_t chInfo32Arr[m_channels];
        memset(chInfo32Arr, 0, m_channels * sizeof(uint32_t));

        for (uint32_t i = 0; i < m_channels; i++) {
            chInfo32Arr[i] = p_info[i];
        }
        uint32_t l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * 4;
        memcpy(&m_buf[l_offset], reinterpret_cast<uint8_t*>(chInfo32Arr), m_channels * 4 * sizeof(uint8_t));
    }

    std::vector<uint16_t> get_chInfo16(uint32_t p_rbId, uint32_t p_chInfo16Id) {
        uint32_t l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * (2 + p_chInfo16Id);
        std::vector<uint16_t> l_chInfo16;
        l_chInfo16.resize(m_channels);
        memcpy(&l_chInfo16[0], &m_buf[l_offset], m_channels * sizeof(uint16_t));
        return l_chInfo16;
    }

    void update_buf() {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = m_totalRows;
        int32Arr[1] = m_totalRbs;
        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    void write_file(std::string filename) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = m_totalRows;
        int32Arr[1] = m_totalRbs;
        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
        saveBin(filename, (char*)&m_buf[0], sizeof(uint8_t) * m_buf.size());
    }

   public:
    uint32_t m_memBytes, m_channels = 0;
    std::vector<uint8_t, alignedAllocator<uint8_t> > m_buf;
};

class ParParam {
   public:
    uint32_t m_memBytes, m_channels, m_totalPars;

    ParParam() = default;
    ParParam(uint32_t p_memBits, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalPars = 0;
    }

    void init(uint32_t p_memBits, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalPars = 0;
    }

    void add_chInfo16(uint32_t* p_info) {
        uint16_t chInfo16Arr[m_channels];
        memset(chInfo16Arr, 0, m_channels * sizeof(uint16_t));

        for (uint32_t i = 0; i < m_channels; i++) {
            chInfo16Arr[i] = (uint16_t)p_info[i];
        }
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_channels * sizeof(uint16_t) / sizeof(uint8_t));
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(chInfo16Arr), m_channels * sizeof(uint16_t));
    }

    void add_chInfo32(uint32_t* p_info) {
        uint32_t chInfo32Arr[m_channels];
        memset(chInfo32Arr, 0, m_channels * sizeof(uint32_t));

        for (uint32_t i = 0; i < m_channels; i++) {
            chInfo32Arr[i] = p_info[i];
        }
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_channels * sizeof(uint32_t) / sizeof(uint8_t));
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(chInfo32Arr), m_channels * sizeof(uint32_t));
    }

    void add_parInfo(uint32_t p_baseColAddr, uint32_t p_colBks, uint32_t p_rows, uint32_t p_nnzs) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));

        int32Arr[0] = p_baseColAddr;
        int32Arr[1] = p_colBks;
        int32Arr[2] = p_rows;
        int32Arr[3] = p_nnzs;
        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_dummyInfo() {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));

        uint32_t old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    uint32_t get_par_offset(uint32_t p_parId) {
        uint32_t l_offset = m_memBytes + p_parId * 8 * m_memBytes;
        return l_offset;
    }

    std::vector<uint32_t> get_parInfo(uint32_t p_parId) {
        uint32_t l_offset = get_par_offset(p_parId);
        l_offset += 4 * m_memBytes;
        std::vector<uint32_t> int32Arr;
        int32Arr.resize(m_memBytes / 4);
        memcpy(&int32Arr[0], &m_buf[l_offset], m_memBytes / 4 * sizeof(uint32_t));
        return int32Arr;
    }

    std::vector<uint16_t> get_chInfo16(uint32_t p_parId, uint32_t p_chInfo16Id) {
        uint32_t l_offset = get_par_offset(p_parId);
        l_offset += m_memBytes * (5 + p_chInfo16Id);
        std::vector<uint16_t> l_chInfo16;
        l_chInfo16.resize(m_channels);
        memcpy(&l_chInfo16[0], &m_buf[l_offset], m_channels * sizeof(uint16_t));
        return l_chInfo16;
    }

    void update_buf() {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = m_totalPars;
        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }
    void write_file(std::string filename) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        int32Arr[0] = m_totalPars;

        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
        saveBin(filename, (char*)&m_buf[0], sizeof(uint8_t) * m_buf.size());
    }

   public:
    std::vector<uint8_t, alignedAllocator<uint8_t> > m_buf;
};

class NnzStore {
   public:
    std::vector<uint32_t> m_totalBks;
    std::vector<uint32_t> m_totalRowIdxBks;
    std::vector<uint32_t> m_totalColIdxBks;
    std::vector<uint32_t> m_totalNnzBks;
    NnzStore() = default;
    NnzStore(uint32_t p_memBits, uint32_t p_parEntries, uint32_t p_accLatency, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        p_parEntries = m_parEntries;
        m_accLatency = p_accLatency;
        m_channels = p_channels;
        m_totalBks.resize(m_channels);
        m_totalRowIdxBks.resize(m_channels);
        m_totalColIdxBks.resize(m_channels);
        m_totalNnzBks.resize(m_channels);
        m_buf.resize(m_channels);
    }

    void init(uint32_t p_memBits, uint32_t p_parEntries, uint32_t p_accLatency, uint32_t p_channels) {
        m_memBytes = p_memBits / 8;
        p_parEntries = m_parEntries;
        m_accLatency = p_accLatency;
        m_channels = p_channels;
        m_totalBks.resize(m_channels);
        m_totalRowIdxBks.resize(m_channels);
        m_totalColIdxBks.resize(m_channels);
        m_totalNnzBks.resize(m_channels);
        m_buf.resize(m_channels);
    }

    void add_dummyInfo(uint32_t p_chId) {
        uint32_t int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
        uint32_t old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_idxArr(uint32_t p_chId, uint32_t* p_idxArr) {
        uint16_t int16Arr[m_memBytes / 2];
        for (uint32_t i = 0; i < m_memBytes / 2; i++) {
            int16Arr[i] = p_idxArr[i];
        }
        uint32_t old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(int16Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_nnzArr(uint32_t p_chId, double* p_nnzArr) {
        double float64Arr[m_memBytes / 8];
        for (uint32_t i = 0; i < m_memBytes / 8; i++) {
            float64Arr[i] = p_nnzArr[i];
        }
        uint32_t old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(float64Arr), m_memBytes * sizeof(uint8_t));
    }

    void update_buf() {
        for (uint32_t i = 0; i < m_channels; i++) {
            assert(m_totalBks[i] == (m_totalRowIdxBks[i] + m_totalColIdxBks[i] + m_totalNnzBks[i]));
            uint32_t int32Arr[m_memBytes / 4];
            memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
            int32Arr[0] = m_totalBks[i];
            int32Arr[1] = m_totalRowIdxBks[i];
            int32Arr[2] = m_totalColIdxBks[i];
            int32Arr[3] = m_totalNnzBks[i];

            memcpy(&m_buf[i][0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
        }
    }
    void write_file(std::string* filenames) {
        for (uint32_t i = 0; i < m_channels; i++) {
            assert(m_totalBks[i] == (m_totalRowIdxBks[i] + m_totalColIdxBks[i] + m_totalNnzBks[i]));
            uint32_t int32Arr[m_memBytes / 4];
            memset(int32Arr, 0, m_memBytes / 4 * sizeof(uint32_t));
            int32Arr[0] = m_totalBks[i];
            int32Arr[1] = m_totalRowIdxBks[i];
            int32Arr[2] = m_totalColIdxBks[i];
            int32Arr[3] = m_totalNnzBks[i];

            memcpy(&m_buf[i][0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
            saveBin(filenames[i], (char*)&m_buf[i][0], sizeof(uint8_t) * m_buf[i].size());
        }
    }

   public:
    uint32_t m_memBytes, m_parEntries, m_accLatency, m_channels;
    std::vector<std::vector<uint8_t, alignedAllocator<uint8_t> > > m_buf;
};

struct CooMat {
    void* m_rowIdxPtr;
    void* m_colIdxPtr;
    void* m_datPtr;
};

struct CooMatInfo {
    std::string m_name;
    uint32_t m_m;
    uint32_t m_n;
    uint32_t m_nnz;
};

struct MatPartition {
    uint32_t m_m, m_n, m_nnz;
    uint32_t m_mPad, m_nPad, m_nnzPad;
    void* m_rbParamPtr;
    uint32_t m_rbParamSize;
    void* m_parParamPtr;
    uint32_t m_parParamSize;
    std::vector<void*> m_nnzValPtr;
    std::vector<uint32_t> m_nnzValSize;
};
#endif

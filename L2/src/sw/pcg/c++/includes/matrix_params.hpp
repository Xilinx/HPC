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

using namespace std;

void readBin(string name, char* mat, unsigned int totalSize) {
    ifstream inFile;
    inFile.open(name, ifstream::binary);
    if (inFile.is_open()) {
        inFile.read((char*)mat, totalSize);
        inFile.close();
    } else {
        cerr << "Could not find " << name << endl;
        exit(1);
    }
}

void saveBin(string name, char* mat, unsigned int totalSize) {
    ofstream outFile(name, ios::binary);
    outFile.write((char*)mat, totalSize);
}

class SparseMatrix {
   public:
    SparseMatrix() {
        m_row_list = nullptr;
        m_col_list = nullptr;
        m_data_list = nullptr;
    }
    SparseMatrix(int m, int n, int nnz) {
        m_m = m;
        m_n = n;
        m_nnz = nnz;
        m_row_list = nullptr;
        m_col_list = nullptr;
        m_data_list = nullptr;
    }

    void load_row(string path) {
        m_row_list = (int*)malloc(m_nnz * sizeof(int));
        readBin(path, (char*)m_row_list, m_nnz * sizeof(int));
        m_minRowId = *(min_element(m_row_list, m_row_list + m_nnz));
    }

    void load_col(string path) {
        m_col_list = (int*)malloc(m_nnz * sizeof(int));
        readBin(path, (char*)m_col_list, m_nnz * sizeof(int));
        m_minColId = *(min_element(m_col_list, m_col_list + m_nnz));
    }

    void load_data(string path) {
        m_data_list = (double*)malloc(m_nnz * sizeof(double));
        readBin(path, (char*)m_data_list, m_nnz * sizeof(double));
    }

    void create_matrix(vector<int> p_row, vector<int> p_col, vector<double> p_data) {
        m_nnz = p_row.size();
        m_row_list = (int*)malloc(m_nnz * sizeof(int));
        m_col_list = (int*)malloc(m_nnz * sizeof(int));
        m_data_list = (double*)malloc(m_nnz * sizeof(double));

        memcpy(m_row_list, &p_row[0], m_nnz * sizeof(int));
        memcpy(m_col_list, &p_col[0], m_nnz * sizeof(int));
        memcpy(m_data_list, &p_data[0], m_nnz * sizeof(double));

        m_minRowId = *(min_element(m_row_list, m_row_list + m_nnz));
        m_minColId = *(min_element(m_col_list, m_col_list + m_nnz));
        int l_maxRowId = *(max_element(m_row_list, m_row_list + m_nnz));
        int l_maxColId = *(max_element(m_col_list, m_col_list + m_nnz));
        m_m = l_maxRowId - m_minRowId + 1;
        m_n = l_maxColId - m_minColId + 1;
    }

    int getRow(int index) { return m_row_list[index]; }

    int* getRows() { return m_row_list; }

    int getCol(int index) { return m_col_list[index]; }

    int* getCols() { return m_col_list; }

    double getData(int index) { return m_data_list[index]; }

    double* getDatas() { return m_data_list; }

    int getM() { return m_m; }

    int getN() { return m_n; }

    int getNnz() { return m_nnz; }

    int getMinRowId() { return m_minRowId; }
    int getMinColId() { return m_minColId; }

    bool sort_by_row() {
        vector<int> idx(m_nnz);
        iota(idx.begin(), idx.end(), 0);

        stable_sort(idx.begin(), idx.end(), [this](int i1, int i2) { return m_col_list[i1] < m_col_list[i2]; });
        stable_sort(idx.begin(), idx.end(), [this](int i1, int i2) { return m_row_list[i1] < m_row_list[i2]; });
        // then sort each part using this index
        int* l_row_list = (int*)malloc(m_nnz * sizeof(int));
        int* l_col_list = (int*)malloc(m_nnz * sizeof(int));
        double* l_data_list = (double*)malloc(m_nnz * sizeof(double));
        for (int i = 0; i < m_nnz; i++) {
            l_row_list[i] = m_row_list[idx[i]];
            l_col_list[i] = m_col_list[idx[i]];
            l_data_list[i] = m_data_list[idx[i]];
        }
        memcpy(m_row_list, l_row_list, m_nnz * sizeof(int));
        memcpy(m_col_list, l_col_list, m_nnz * sizeof(int));
        memcpy(m_data_list, l_data_list, m_nnz * sizeof(double));
        free(l_row_list);
        free(l_col_list);
        free(l_data_list);
    }

    bool sort_by_col() {
        vector<int> idx(m_nnz);
        iota(idx.begin(), idx.end(), 0);
        stable_sort(idx.begin(), idx.end(), [this](int i1, int i2) { return m_row_list[i1] < m_row_list[i2]; });
        stable_sort(idx.begin(), idx.end(), [this](int i1, int i2) { return m_col_list[i1] < m_col_list[i2]; });
        // then sort each part using this index
        int* l_row_list = (int*)malloc(m_nnz * sizeof(int));
        int* l_col_list = (int*)malloc(m_nnz * sizeof(int));
        double* l_data_list = (double*)malloc(m_nnz * sizeof(double));
        for (int i = 0; i < m_nnz; i++) {
            l_row_list[i] = m_row_list[idx[i]];
            l_col_list[i] = m_col_list[idx[i]];
            l_data_list[i] = m_data_list[idx[i]];
        }
        memcpy(m_row_list, l_row_list, m_nnz * sizeof(int));
        memcpy(m_col_list, l_col_list, m_nnz * sizeof(int));
        memcpy(m_data_list, l_data_list, m_nnz * sizeof(double));
        free(l_row_list);
        free(l_col_list);
        free(l_data_list);
    }

   private:
    int m_m, m_n, m_nnz;
    int *m_row_list, *m_col_list;
    double* m_data_list;
    int m_minRowId, m_minColId;
};

class RowBlockParam {
   public:
    int m_totalRows, m_totalRbs;

    RowBlockParam() = default;

    RowBlockParam(int p_memBits, int p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalRows = 0;
        m_totalRbs = 0;
    }

    void init(int p_memBits, int p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalRows = 0;
        m_totalRbs = 0;
    }

    void add_rbIdxInfo(int p_minRowId, int p_minColId, int p_numCols, int p_numPars) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int32Arr[0] = p_minRowId;
        int32Arr[1] = p_minColId;
        int32Arr[2] = p_numCols;
        int32Arr[3] = p_numPars;
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(int));
    }

    void add_rbSizeInfo(int p_numRows, int p_numNnzs) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int32Arr[0] = p_numRows;
        int32Arr[1] = p_numNnzs;
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(int));
    }

    void add_dummyInfo() {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes / 4 * sizeof(int));
    }

    int get_rb_offset(int p_rbId) {
        int l_offset = m_memBytes;
        l_offset += p_rbId * m_memBytes * 8;
        return l_offset;
    }

    vector<int> get_rbInfo(int p_rbId, int p_rbInfoId) {
        int l_size = m_memBytes / 4;
        int l_offset = get_rb_offset(p_rbId);
        l_offset += p_rbInfoId * m_memBytes;

        vector<int> l_buf;
        l_buf.resize(l_size);
        memcpy(&l_buf[0], &m_buf[l_offset], l_size * sizeof(int));
        return l_buf;
    }

    void set_numPars(int p_rbId, int p_numPars) {
        int l_offset = get_rb_offset(p_rbId);
        int l_size = m_memBytes / 4;

        vector<int> int32Arr;
        int32Arr.resize(l_size);
        memcpy(&int32Arr[0], &m_buf[l_offset], l_size * sizeof(int));
        int32Arr[3] = p_numPars;
        memcpy(&m_buf[l_offset], &int32Arr[0], l_size * sizeof(int));
    }

    void set_numNnzs(int p_rbId, int p_numNnzs) {
        int l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes;
        int l_size = m_memBytes / 4;
        vector<int> int32Arr;
        int32Arr.resize(l_size);
        memcpy(&int32Arr[0], &m_buf[l_offset], l_size * sizeof(int));
        int32Arr[1] = p_numNnzs;
        memcpy(&m_buf[l_offset], &int32Arr[0], l_size * sizeof(int));
    }

    void set_chInfo16(int p_rbId, int p_chInfo16Id, int* p_info) {
        uint16_t chInfo16Arr[m_channels];
        memset(chInfo16Arr, 0, m_channels * sizeof(uint16_t));

        for (int i = 0; i < m_channels; i++) {
            chInfo16Arr[i] = p_info[i];
        }
        int l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * (2 + p_chInfo16Id);
        memcpy(&m_buf[l_offset], reinterpret_cast<uint8_t*>(chInfo16Arr), m_memBytes * sizeof(uint8_t));
    }

    void set_chInfo32(int p_rbId, int* p_info) {
        int chInfo32Arr[m_channels];
        memset(chInfo32Arr, 0, m_channels * sizeof(int));

        for (int i = 0; i < m_channels; i++) {
            chInfo32Arr[i] = p_info[i];
        }
        int l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * 4;
        memcpy(&m_buf[l_offset], reinterpret_cast<uint8_t*>(chInfo32Arr), m_channels * 4 * sizeof(uint8_t));
    }

    vector<uint16_t> get_chInfo16(int p_rbId, int p_chInfo16Id) {
        int l_offset = get_rb_offset(p_rbId);
        l_offset += m_memBytes * (2 + p_chInfo16Id);
        vector<uint16_t> l_chInfo16;
        l_chInfo16.resize(m_channels);
        memcpy(&l_chInfo16[0], &m_buf[l_offset], m_channels * sizeof(uint16_t));
        return l_chInfo16;
    }

    void write_file(string filename) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int32Arr[0] = m_totalRows;
        int32Arr[1] = m_totalRbs;
        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
        saveBin(filename, (char*)&m_buf[0], sizeof(uint8_t) * m_buf.size());
    }

   private:
    int m_memBytes, m_channels = 0;
    vector<uint8_t> m_buf;
};

class ParParam {
   public:
    int m_memBytes, m_channels, m_totalPars;

    ParParam() = default;
    ParParam(int p_memBits, int p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalPars = 0;
    }

    void init(int p_memBits, int p_channels) {
        m_memBytes = p_memBits / 8;
        m_channels = p_channels;
        m_totalPars = 0;
    }

    void add_chInfo16(int* p_info) {
        uint16_t chInfo16Arr[m_channels];
        memset(chInfo16Arr, 0, m_channels * sizeof(uint16_t));

        for (int i = 0; i < m_channels; i++) {
            chInfo16Arr[i] = (uint16_t)p_info[i];
        }
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_channels * sizeof(uint16_t) / sizeof(uint8_t));
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(chInfo16Arr), m_channels * sizeof(uint16_t));
    }

    void add_chInfo32(int* p_info) {
        int chInfo32Arr[m_channels];
        memset(chInfo32Arr, 0, m_channels * sizeof(int));

        for (int i = 0; i < m_channels; i++) {
            chInfo32Arr[i] = (uint16_t)p_info[i];
        }
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_channels * sizeof(uint32_t) / sizeof(uint8_t));
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(chInfo32Arr), m_channels * sizeof(uint32_t));
    }

    void add_parInfo(int p_baseColAddr, int p_colBks, int p_rows, int p_nnzs) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));

        int32Arr[0] = p_baseColAddr;
        int32Arr[1] = p_colBks;
        int32Arr[2] = p_rows;
        int32Arr[3] = p_nnzs;
        int old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_dummyInfo() {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));

        int old_size = m_buf.size();
        m_buf.resize(old_size + m_memBytes);
        memcpy(&m_buf[old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    int get_par_offset(int p_parId) {
        int l_offset = m_memBytes + p_parId * 8 * m_memBytes;
        return l_offset;
    }

    vector<int> get_parInfo(int p_parId) {
        int l_offset = get_par_offset(p_parId);
        l_offset += 4 * m_memBytes;
        vector<int> int32Arr;
        int32Arr.resize(m_memBytes / 4);
        memcpy(&int32Arr[0], &m_buf[l_offset], m_memBytes / 4 * sizeof(int));
        return int32Arr;
    }

    vector<uint16_t> get_chInfo16(int p_parId, int p_chInfo16Id) {
        int l_offset = get_par_offset(p_parId);
        l_offset += m_memBytes * (5 + p_chInfo16Id);
        vector<uint16_t> l_chInfo16;
        l_chInfo16.resize(m_channels);
        memcpy(&l_chInfo16[0], &m_buf[l_offset], m_channels * sizeof(uint16_t));
        return l_chInfo16;
    }

    void write_file(string filename) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int32Arr[0] = m_totalPars;

        memcpy(&m_buf[0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
        saveBin(filename, (char*)&m_buf[0], sizeof(uint8_t) * m_buf.size());
    }

   private:
    vector<uint8_t> m_buf;
};

class NnzStore {
   public:
    int* m_totalBks;
    int* m_totalRowIdxBks;
    int* m_totalColIdxBks;
    int* m_totalNnzBks;
    NnzStore() = default;
    NnzStore(int p_memBits, int p_parEntries, int p_accLatency, int p_channels) {
        m_memBytes = p_memBits / 8;
        p_parEntries = m_parEntries;
        m_accLatency = p_accLatency;
        m_channels = p_channels;
        m_totalBks = (int*)malloc(m_channels * sizeof(int));
        m_totalRowIdxBks = (int*)malloc(m_channels * sizeof(int));
        m_totalColIdxBks = (int*)malloc(m_channels * sizeof(int));
        m_totalNnzBks = (int*)malloc(m_channels * sizeof(int));
        m_buf.resize(m_channels);
    }

    void init(int p_memBits, int p_parEntries, int p_accLatency, int p_channels) {
        m_memBytes = p_memBits / 8;
        p_parEntries = m_parEntries;
        m_accLatency = p_accLatency;
        m_channels = p_channels;
        m_totalBks = (int*)malloc(m_channels * sizeof(int));
        m_totalRowIdxBks = (int*)malloc(m_channels * sizeof(int));
        m_totalColIdxBks = (int*)malloc(m_channels * sizeof(int));
        m_totalNnzBks = (int*)malloc(m_channels * sizeof(int));
        m_buf.resize(m_channels);
    }

    void add_dummyInfo(int p_chId) {
        int int32Arr[m_memBytes / 4];
        memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
        int old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_idxArr(int p_chId, int* p_idxArr) {
        uint16_t int16Arr[m_memBytes / 2];
        for (int i = 0; i < m_memBytes / 2; i++) {
            int16Arr[i] = p_idxArr[i];
        }
        int old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(int16Arr), m_memBytes * sizeof(uint8_t));
    }

    void add_nnzArr(int p_chId, double* p_nnzArr) {
        double float64Arr[m_memBytes / 8];
        for (int i = 0; i < m_memBytes / 8; i++) {
            float64Arr[i] = p_nnzArr[i];
        }
        int old_size = m_buf[p_chId].size();
        m_buf[p_chId].resize(old_size + m_memBytes);
        memcpy(&m_buf[p_chId][old_size], reinterpret_cast<uint8_t*>(float64Arr), m_memBytes * sizeof(uint8_t));
    }

    void write_file(string* filenames) {
        for (int i = 0; i < m_channels; i++) {
            assert(m_totalBks[i] == (m_totalRowIdxBks[i] + m_totalColIdxBks[i] + m_totalNnzBks[i]));
            int int32Arr[m_memBytes / 4];
            memset(int32Arr, 0, m_memBytes / 4 * sizeof(int));
            int32Arr[0] = m_totalBks[i];
            int32Arr[1] = m_totalRowIdxBks[i];
            int32Arr[2] = m_totalColIdxBks[i];
            int32Arr[3] = m_totalNnzBks[i];

            memcpy(&m_buf[i][0], reinterpret_cast<uint8_t*>(int32Arr), m_memBytes * sizeof(uint8_t));
            saveBin(filenames[i], (char*)&m_buf[i][0], sizeof(uint8_t) * m_buf[i].size());
        }
    }

   private:
    int m_memBytes, m_parEntries, m_accLatency, m_channels;
    vector<vector<uint8_t> > m_buf;
};

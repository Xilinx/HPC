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
#include <string>
#include <chrono>
#include <fstream>
#include "matrix_params.hpp"
#include "signature.hpp"
#include "binFiles.hpp"
#include "gen_signature.hpp"

using namespace std;


CooMatInfo loadMatInfo(string path) {
    ifstream l_info(path + "infos.txt");
    vector<string> infos;
    for (string line; getline(l_info, line);) {
        infos.push_back(line);
    }
    CooMatInfo l_matInfo;
    l_matInfo.m_name = infos[0];
    l_matInfo.m_m = stoi(infos[1]);
    l_matInfo.m_n = stoi(infos[2]);
    l_matInfo.m_nnz = stoi(infos[3]);
    return l_matInfo;
} 

void loadMat(string path, CooMatInfo& p_matInfo, CooMat& p_mat) {
    uint32_t l_idxBytes = sizeof(uint32_t)*p_matInfo.m_nnz;
    uint32_t l_datBytes = sizeof(double)*p_matInfo.m_nnz;
    string l_rowIdxFileName = path+"row.bin";
    string l_colIdxFileName = path+"col.bin";
    string l_datFileName = path+"data.bin";
    readBin(l_rowIdxFileName, p_mat.m_rowIdxPtr, l_idxBytes);
    readBin(l_colIdxFileName, p_mat.m_colIdxPtr, l_idxBytes);
    readBin(l_datFileName, p_mat.m_datPtr, l_datBytes);
    
}

void storeMatPar(string path, MatPartition& p_matPar) {
    string l_rbParamFileName(path + "/rbParam.dat");
    saveBin(l_rbParamFileName, p_matPar.m_rbParamPtr, p_matPar.m_rbParamSize);
    string l_parParamFileName(path + "/parParam.dat");
    saveBin(l_parParamFileName, p_matPar.m_parParamPtr, p_matPar.m_parParamSize);

    string l_nnzFileNames[SPARSE_hbmChannels];
    for (int i = 0; i < SPARSE_hbmChannels; i++) {
        l_nnzFileNames[i] = path + "/nnzVal_" + to_string(i) + ".dat";
        saveBin(l_nnzFileNames[i], p_matPar.m_nnzValPtr[i], p_matPar.m_nnzValSize[i]);
    }
    string l_infoFileName(path + "/info.dat");
    int int32Arr[6];
    memset(int32Arr, 0, 6 * sizeof(int));
    int32Arr[0] = p_matPar.m_m;
    int32Arr[1] = p_matPar.m_n;
    int32Arr[2] = p_matPar.m_nnz;
    int32Arr[3] = p_matPar.m_mPad;
    int32Arr[4] = p_matPar.m_nPad;
    int32Arr[5] = p_matPar.m_nnzPad;
    ofstream outFile(l_infoFileName, ios::binary);
    outFile.write((char*)&int32Arr[0], sizeof(int) * 6);
    outFile.close();
}

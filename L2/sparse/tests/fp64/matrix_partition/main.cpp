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

#include "gen_signature.hpp"

using namespace std;

int main(int argc, char** argv) {
    TimePointType l_timer[2];
    int arg = 0;
    string dataPath = argv[++arg];
    int l_runs = atoi(argv[++arg]);
    int l_update = atoi(argv[++arg]);
    string l_dataPath = dataPath + "/"; 
    xf::sparse::CooMatInfo l_matInfo = xf::sparse::loadMatInfo(l_dataPath);
    std::vector<uint32_t> l_rowIdx(l_matInfo.m_nnz);
    std::vector<uint32_t> l_colIdx(l_matInfo.m_nnz);
    std::vector<SPARSE_dataType> l_data(l_matInfo.m_nnz);
    xf::sparse::SpmPar<SPARSE_dataType> l_spmPar(SPARSE_parEntries, SPARSE_accLatency, SPARSE_hbmChannels, SPARSE_maxRows, SPARSE_maxCols,
           SPARSE_hbmMemBits);
    for (unsigned int i=0; i<l_runs; ++i) {
        //string l_dataPath = dataPath + "/" + to_string(i) + "/"; 
        readBin(l_dataPath + "row.bin", l_rowIdx.data(), l_matInfo.m_nnz * sizeof(uint32_t));
        readBin(l_dataPath + "col.bin", l_colIdx.data(), l_matInfo.m_nnz * sizeof(uint32_t));
        readBin(l_dataPath + "data.bin", l_data.data(), l_matInfo.m_nnz * sizeof(SPARSE_dataType));

      
        l_timer[0] = chrono::high_resolution_clock::now();
        xf::sparse::MatPartition l_matPar;
         if ((i == 0) || (l_update == 0)) {
            l_matPar = l_spmPar.partitionCooMat(l_matInfo.m_m, l_matInfo.m_n, l_matInfo.m_nnz, l_rowIdx.data(),
                                                         l_colIdx.data(), l_data.data(), 0); //partition sparse matrix with C storage type
        }
        else {
            l_matPar = l_spmPar.updateMat(l_data.data());
        }
        showTimeData("INFO: Matrix partition time: ", l_timer[0], l_timer[1]);
        storeMatPar(l_dataPath, l_matPar);
        printf("INFO: matrix %s partiton done.\n", l_matInfo.m_name.c_str());
        printf("      Original m, n, nnzs = %d, %d, %d\n", l_matPar.m_m, l_matPar.m_n, l_matPar.m_nnz);
        printf("      After padding m, n, nnzs = %d, %d, %d\n", l_matPar.m_mPad, l_matPar.m_nPad, l_matPar.m_nnzPad);
        printf("      Padding overhead is %f\n", (double)(l_matPar.m_nnzPad - l_matPar.m_nnz) / l_matPar.m_nnz);
    }
    return EXIT_SUCCESS;
}

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

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include "matrix_params.hpp"
#include "signature.hpp"
#include "binFiles.hpp"
#include "gen_signature.hpp"

namespace xf {
namespace sparse {

CooMatInfo loadMatInfo(std::string path) {
    std::ifstream l_info(path + "infos.txt");
    std::vector<std::string> infos;
    for (std::string line; getline(l_info, line);) {
        infos.push_back(line);
    }
    CooMatInfo l_matInfo;
    l_matInfo.m_name = infos[0];
    l_matInfo.m_m = stoi(infos[1]);
    l_matInfo.m_n = stoi(infos[2]);
    l_matInfo.m_nnz = stoi(infos[3]);
    return l_matInfo;
}

void storeMatPar(std::string path, MatPartition& p_matPar) {
    std::string l_rbParamFileName(path + "/rbParam.dat");
    saveBin(l_rbParamFileName, p_matPar.m_rbParamPtr, p_matPar.m_rbParamSize);
    std::string l_parParamFileName(path + "/parParam.dat");
    saveBin(l_parParamFileName, p_matPar.m_parParamPtr, p_matPar.m_parParamSize);

    std::string l_nnzFileNames[SPARSE_hbmChannels];
    for (int i = 0; i < SPARSE_hbmChannels; i++) {
        l_nnzFileNames[i] = path + "/nnzVal_" + std::to_string(i) + ".dat";
        saveBin(l_nnzFileNames[i], p_matPar.m_nnzValPtr[i], p_matPar.m_nnzValSize[i]);
    }
    std::string l_infoFileName(path + "/info.dat");
    int int32Arr[6];
    memset(int32Arr, 0, 6 * sizeof(int));
    int32Arr[0] = p_matPar.m_m;
    int32Arr[1] = p_matPar.m_n;
    int32Arr[2] = p_matPar.m_nnz;
    int32Arr[3] = p_matPar.m_mPad;
    int32Arr[4] = p_matPar.m_nPad;
    int32Arr[5] = p_matPar.m_nnzPad;
    std::ofstream outFile(l_infoFileName, std::ios::binary);
    outFile.write((char*)&int32Arr[0], sizeof(int) * 6);
    outFile.close();
}

}
}

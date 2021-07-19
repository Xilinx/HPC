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

//#include <algorithm>
//#include <cstring>
#include <iostream>
#include <string>
//#include <thread>
//#include <unistd.h>
#include <vector>
#include <cassert>

// This file is required for OpenCL C++ wrapper APIs
#include "pcg.hpp"
//#include "utils.hpp"

using CG_dataType = double;

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> [device id]"
                  << std::endl;
        return EXIT_FAILURE;
    }
    int l_idx = 1;
    std::string binaryFile = argv[l_idx++];
    int l_deviceId = 0;
    if (argc > l_idx) {
        std::string l_option = argv[l_idx++];
        l_deviceId = stoi(l_option);
    }
    if (argc > l_idx) l_deviceId = atoi(argv[l_idx++]);


    unsigned l_maxIter = 1;
    CG_dataType l_tol = 1e-12;
    uint32_t m_m = 10;
    uint32_t m_n = 10;
    uint32_t m_nnz = 10;
    assert(m_m == m_n);
    std::vector<CG_dataType> h_x(m_m);
    std::vector<uint32_t> l_rowIdx(m_nnz);
    std::vector<uint32_t> l_colIdx(m_nnz);
    std::vector<CG_dataType> l_data(m_nnz);
    std::vector<CG_dataType> l_b(m_m);
    std::vector<CG_dataType> l_diagA(m_m);
    // TODO: fill the vectors

    struct xilinx_apps::pcg::Options l_option = {l_deviceId, xilinx_apps::pcg::XString(binaryFile)};
    xilinx_apps::pcg::PCG<CG_dataType> l_pcg(l_option);
    l_pcg.setCooMat(m_m, m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data());
    xilinx_apps::pcg::Results<CG_dataType> l_res;
    l_pcg.setVec(m_m, l_b.data(), l_diagA.data());
    l_res = l_pcg.run(l_maxIter, l_tol);

    //if (l_pcg.updateMat(m_m, m_nnz, l_data.data()) != 0) {
    //    return EXIT_FAILURE;
    //}
    l_pcg.setCooMat(m_m, m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data());
    l_pcg.setVec(m_m, l_b.data(), l_diagA.data());
    l_res = l_pcg.run(l_maxIter, l_tol);

    std::vector<uint32_t> l_info = l_pcg.getMatInfo();
    float l_padRatio = (float)(l_info[5]) / (float)(l_info[2]);

    xilinx_apps::pcg::CgVector l_resVec = l_pcg.getRes();
    (void) l_resVec;  // TODO: determine what to do with the output


    std::cout << "DATA_CSV:,dim, original NNZs, padded dim, padded NNZs, padding ratio, ";
    std::cout << std::endl;
    std::cout << "DATA_CSV:," << l_info[0] << "," << l_info[2];
    std::cout << "," << l_info[4] << "," << l_info[5] << "," << l_padRatio;
    std::cout << std::endl;
    return EXIT_SUCCESS;
}

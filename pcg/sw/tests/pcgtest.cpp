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

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <cassert>

#include "pcg.h"
#include "sw/utils.hpp"
#include "sw/fp64/matrix_params.hpp"
#include "sw/fp64/gen_signature.hpp"

#ifndef CG_dataType
using CG_dataType = double;
#endif

int main(int argc, char** argv) {
    if (argc < 6 || argc > 8) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <Max Iteration> <Tolerence> <data_path> "
                                             "<mtx_name> <number_of_runs> [--debug] [device id]"
                  << std::endl;
        return EXIT_FAILURE;
    }
    int l_idx = 1;
    std::string binaryFile = argv[l_idx++];
    int l_maxIter = atoi(argv[l_idx++]);
    CG_dataType l_tolerance = atof(argv[l_idx++]);
    std::string l_datPath = argv[l_idx++];
    std::string l_mtxName = argv[l_idx++];
    int l_numRuns = atoi(argv[l_idx++]);
    int l_deviceId = 0;
    bool l_debug = false;
    if (argc > l_idx) {
        std::string l_option = argv[l_idx++];
        if (l_option == "--debug")
            l_debug = true;
        else
            l_deviceId = stoi(l_option);
    }
    if (argc > l_idx) l_deviceId = atoi(argv[l_idx++]);

    std::string l_datFilePath = l_datPath + "/" + l_mtxName;

    CooMatInfo l_matInfo = loadMatInfo(l_datFilePath + "/");
    assert(l_matInfo.m_m == l_matInfo.m_n);
    std::vector<CG_dataType> h_x(l_matInfo.m_m);
    std::vector<uint32_t> l_rowIdx(l_matInfo.m_nnz);
    std::vector<uint32_t> l_colIdx(l_matInfo.m_nnz);
    std::vector<CG_dataType> l_data(l_matInfo.m_nnz);
    std::vector<CG_dataType> l_b(l_matInfo.m_m);
    std::vector<CG_dataType> l_x(l_matInfo.m_m);
    std::vector<CG_dataType> l_diagA(l_matInfo.m_m);
    readBin(l_datFilePath + "/row.bin", l_rowIdx.data(), l_matInfo.m_nnz * sizeof(uint32_t));
    readBin(l_datFilePath + "/col.bin", l_colIdx.data(), l_matInfo.m_nnz * sizeof(uint32_t));
    readBin(l_datFilePath + "/data.bin", l_data.data(), l_matInfo.m_nnz * sizeof(CG_dataType));
    readBin(l_datFilePath + "/A_diag.mat", l_diagA.data(), l_matInfo.m_m * sizeof(CG_dataType));
    readBin(l_datFilePath + "/b.mat", l_b.data(), l_matInfo.m_m * sizeof(CG_dataType));

    TimePointType l_timer[8];

    l_timer[0] = std::chrono::high_resolution_clock::now();
//    struct xilinx_apps::pcg::Options l_option = {l_deviceId, xilinx_apps::pcg::XString(binaryFile)};
//    xilinx_apps::pcg::PCG<CG_dataType> l_pcg(l_option);
    void *pHandle = create_JPCG_handle(l_deviceId, binaryFile.c_str());
    showTimeData("FPGA configuration time: ", l_timer[0], l_timer[1]);
//    l_pcg.setCooMat(l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data());
    double l_mat_partition_time = 0;
    showTimeData("Matrix partition and transmission time: ", l_timer[1], l_timer[2], &l_mat_partition_time);
    double l_runTime = 1;
    double l_h2d_time = 0;
//    xilinx_apps::pcg::Results<CG_dataType> l_res;
//    l_pcg.setVec(l_matInfo.m_m, l_b.data(), l_diagA.data());
    showTimeData("Vector initialization & transmission time: ", l_timer[2], l_timer[3], &l_h2d_time);
//    l_res = l_pcg.run(l_maxIter, l_tolerance);
    uint32_t numIterations = 0;
    double residual = 0.0;
    JPCG_coo(pHandle, JPCG_MODE_FULL, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual);
    showTimeData("PCG run time: ", l_timer[3], l_timer[4], &l_runTime);
    
    for (int i = 1; i < l_numRuns; ++i) {
        l_timer[0] = std::chrono::high_resolution_clock::now();
//        if (l_pcg.updateMat(l_matInfo.m_m, l_matInfo.m_nnz, l_data.data()) != 0) {
//            return EXIT_FAILURE;
//        }
        showTimeData("Matrix update time: ", l_timer[0], l_timer[1]);
//        l_pcg.setVec(l_matInfo.m_m, l_b.data(), l_diagA.data());
        showTimeData("Vector initialization & transmission time: ", l_timer[1], l_timer[2], &l_h2d_time);
//        l_res = l_pcg.run(l_maxIter, l_tolerance);
        JPCG_coo(pHandle, JPCG_MODE_KEEP_NZ_LAYOUT, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(),
            l_data.data(), l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual);
        showTimeData("PCG run time: ", l_timer[2], l_timer[3], &l_runTime);
    }

#ifdef TODO    
    std::vector<uint32_t> l_info = l_pcg.getMatInfo();
    float l_padRatio = (float)(l_info[5]) / (float)(l_info[2]);

    if (l_debug) {
        xilinx_apps::pcg::CgVector l_resVec = l_pcg.getRes();
        saveBin(l_datFilePath + "/rk.dat", l_resVec.h_rk, l_matInfo.m_m * sizeof(CG_dataType));
        saveBin(l_datFilePath + "/xk.dat", l_resVec.h_xk, l_matInfo.m_m * sizeof(CG_dataType));
    }
#endif

    int err = 0;
    readBin(l_datFilePath + "/x.mat", h_x.data(), l_matInfo.m_m * sizeof(CG_dataType));
//    compare<CG_dataType>(l_matInfo.m_m, h_x.data(), (CG_dataType*)(l_res.m_x), err, l_debug);
    compare<CG_dataType>(l_matInfo.m_m, h_x.data(), l_x.data(), err, l_debug);

    std::cout << "DATA_CSV:,matrix_name, dim, original NNZs, padded dim, padded NNZs, padding ratio, ";
    std::cout
        << "num of iterations, mat partition time[ms], H2D time[ms], total run time[ms], time[ms]/run, num_mismatches";
    std::cout << std::endl;
    
#ifdef TODO
    std::cout << "DATA_CSV:," << l_mtxName << "," << l_info[0] << "," << l_info[2];
    std::cout << "," << l_info[4] << "," << l_info[5] << "," << l_padRatio;
    std::cout << "," << l_res.m_nIters + 1 << "," << l_mat_partition_time << "," << l_h2d_time << "," << l_runTime
              << "," << (float)l_runTime / (l_res.m_nIters + 1);
    std::cout << "," << err << std::endl;
#endif
    
    if (err == 0) {
        std::cout << "Test pass!" << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cout << "Test failed! There are in total " << err << " mismatches in the solution." << std::endl;
        return EXIT_FAILURE;
    }
}

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
    bool l_debug = false;
    if (argc > l_idx) {
        std::string l_option = argv[l_idx++];
        if (l_option == "--debug")
            l_debug = true;
    }

    std::string l_datFilePath = l_datPath + "/" + l_mtxName;

    xf::sparse::CooMatInfo l_matInfo = xf::sparse::loadMatInfo(l_datFilePath + "/");
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

    XJPCG_Handle_t *pHandle = 0;
    uint32_t numIterations = 0;
    double residual = 0.0;

    /******************/
    /* pressure tests */
    /******************/
    XJPCG_Status_t l_stat;
    
    l_stat = xJPCG_cooSolver(pHandle, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_DEFAULT);
    assert(l_stat == XJPCG_STATUS_NOT_INITIALIZED);

    l_stat = xJPCG_createHandle(&pHandle, binaryFile.c_str());
    assert(l_stat == XJPCG_STATUS_SUCCESS);
    xJPCG_cooSolver(pHandle, 0, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_DEFAULT);
    assert(l_stat == XJPCG_STATUS_INVALID_VALUE);

    l_stat = xJPCG_cooSolver(pHandle, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_KEEP_NZ_LAYOUT);
    assert(l_stat == XJPCG_STATUS_INVALID_VALUE);

    l_stat = xJPCG_cooSolver(pHandle, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), nullptr,
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_DEFAULT);
    assert(l_stat == XJPCG_STATUS_INVALID_VALUE);

    /******************/
    /* solver tests */
    /******************/
    l_stat = xJPCG_cooSolver(pHandle, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
        l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_DEFAULT);
    assert(l_stat == XJPCG_STATUS_SUCCESS);
    
    for (int i = 1; i < l_numRuns; ++i) {
        l_stat = xJPCG_cooSolver(pHandle, l_matInfo.m_m, l_matInfo.m_nnz, l_rowIdx.data(), l_colIdx.data(), l_data.data(),
            l_diagA.data(), l_b.data(), l_x.data(), l_maxIter, l_tolerance, &numIterations, &residual,  XJPCG_MODE_KEEP_NZ_LAYOUT);
        assert(l_stat == XJPCG_STATUS_SUCCESS);
    }

    int err = 0;
    readBin(l_datFilePath + "/x.mat", h_x.data(), l_matInfo.m_m * sizeof(CG_dataType));
    compare<CG_dataType>(l_matInfo.m_m, h_x.data(), l_x.data(), err, l_debug);

    XJPCG_Metric_t metric;
    xJPCG_getMetrics(pHandle, &metric);
    xJPCG_destroyHandle(pHandle);

    std::cout << "DATA_CSV:, matrix_name, dim, NNZs, num of iterations, JPCG residual, num_mismatches, solver time [s]" << std::endl;
    std::cout << "DATA_CSV:, " << l_matInfo.m_name << ", " << l_matInfo.m_m << ", ";
    std::cout << l_matInfo.m_nnz << ", " << numIterations << ", ";
    std::cout << residual  << ", " << err << ", ";
    std::cout << metric.m_solver << std::endl;
    
    
    if ((err == 0) || (numIterations == uint32_t(l_maxIter))){
        std::cout << "Test pass!" << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cout << "Test failed! There are in total " << err << " mismatches in the solution." << std::endl;
        return EXIT_FAILURE;
    }
}

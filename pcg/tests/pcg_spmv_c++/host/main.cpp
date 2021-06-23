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
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <cassert>

// This file is required for OpenCL C++ wrapper APIs
#include "pcg.hpp"
#include "utils.hpp"

using namespace std;

template <typename T>
using host_buffer_t = vector<T, aligned_allocator<T> >;

int main(int argc, char** argv) {
    if (argc < 6 || argc > 8) {
        cout << "Usage: " << argv[0] << " <XCLBIN File> <Max Iteration> <Tolerence> <data_path> "
                                        "<mtx_name> [--debug] [device id]"
             << endl;
        return EXIT_FAILURE;
    }
    int l_idx = 1;
    string binaryFile = argv[l_idx++];
    int l_maxIter = atoi(argv[l_idx++]);
    CG_dataType l_tol = atof(argv[l_idx++]);
    string l_datPath = argv[l_idx++];
    string l_mtxName = argv[l_idx++];
    int l_deviceId = 0;
    bool l_debug = false;
    if (argc > l_idx) {
        string l_option = argv[l_idx++];
        if (l_option == "--debug")
            l_debug = true;
        else
            l_deviceId = stoi(l_option);
    }
    if (argc > l_idx) l_deviceId = atoi(argv[l_idx++]);

    string l_datFilePath = l_datPath + "/" + l_mtxName;

    CooMatInfo l_matInfo = loadMatInfo(l_datFilePath + "/");
    assert(l_matInfo.m_m == l_matInfo.m_n);
    host_buffer_t<CG_dataType> h_x(l_matInfo.m_m);
    TimePointType l_timer[8];
    PCG<CG_dataType, CG_parEntries, CG_instrBytes, SPARSE_accLatency, SPARSE_hbmChannels, SPARSE_maxRows, SPARSE_maxCols, SPARSE_hbmMemBits> l_pcg;

    CooMat l_mat = l_pcg.allocMat(l_matInfo.m_m, l_matInfo.m_n, l_matInfo.m_nnz);
    l_pcg.allocVec(l_matInfo.m_m);
    CgInputVec l_inVecs = l_pcg.getInputVec();
    
    loadMat(l_datFilePath + "/", l_matInfo, l_mat);
    readBin(l_datFilePath + "/A_diag.mat", l_inVecs.h_diag, l_inVecs.vecBytes);
    readBin(l_datFilePath + "/b.mat", l_inVecs.h_b, l_inVecs.vecBytes);

    l_timer[0] = chrono::high_resolution_clock::now();
    l_pcg.partitionMat();
    double l_mat_partition_time = showTimeData("Matrix partition time: ", l_timer[0], l_timer[1]);
    l_pcg.initVec();
    showTimeData("Vector initialization time: ", l_timer[1], l_timer[2]);
    l_pcg.initDev(l_deviceId, binaryFile);
    showTimeData("FPGA configuration time: ", l_timer[2], l_timer[3]);
    //l_pcg.setDat();
    l_pcg.setMat();
    showTimeData("Send Mat time: ", l_timer[3], l_timer[4]);
    l_pcg.setVec();
    showTimeData("Send Vec time: ", l_timer[4], l_timer[5]);
    l_pcg.setInstr(l_maxIter, l_tol);
    double l_h2d_time = showTimeData("Host to device data transfer time: ", l_timer[3], l_timer[4]);
    double l_runTime = 1;
    l_pcg.run();
    CgVector l_resVec = l_pcg.getRes();
    showTimeData("PCG run time: ", l_timer[6], l_timer[7], &l_runTime);
    CgInstr l_instr = l_pcg.getInstr();
    void* l_xk = l_resVec.h_xk;
    void* l_rk = l_resVec.h_rk;
    xf::hpc::MemInstr<CG_instrBytes> l_memInstr;
    xf::hpc::cg::CGSolverInstr<CG_dataType> l_cgInstr;
    int lastIter = 0;
    uint64_t finalClock = 0;
    for (int i = 0; i < l_maxIter; i++) {
        lastIter = i;
        l_cgInstr.load((uint8_t*)(l_instr.h_instr) + (i + 1) * CG_instrBytes, l_memInstr);
        if (l_debug) {
            cout << l_cgInstr << endl;
        }
        if (l_cgInstr.getMaxIter() == 0) {
            break;
        }
        finalClock = l_cgInstr.getClock();
    }
    vector<uint32_t> l_info = l_pcg.getMatInfo();
    cout << "HW execution time is: " << finalClock * HW_CLK * 1e3<< "msec." << endl;
    float l_padRatio = (float)(l_info[5]) / (float)(l_info[2]);

    if (l_debug) {
        saveBin(l_datFilePath + "/rk.dat", l_rk, l_matInfo.m_m * sizeof(CG_dataType));
        saveBin(l_datFilePath + "/xk.dat", l_xk, l_matInfo.m_m * sizeof(CG_dataType));
    }

    int err = 0;
    readBin(l_datFilePath + "/x.mat", l_matInfo.m_m * sizeof(CG_dataType), h_x);
    //compare(h_x.size(), h_x.data(), h_xk.data(), err, l_debug);
    compare<CG_dataType>(l_matInfo.m_m, h_x.data(), (CG_dataType*)(l_xk), err, l_debug);
    
    cout << "DATA_CSV:,matrix_name, dim, original NNZs, padded dim, padded NNZs, padding ratio, ";
    cout << "num of iterations, mat partition time[ms], H2D time[ms], total run time[ms], time[ms]/run, num_mismatches";
    cout << endl;
    cout << "DATA_CSV:," << l_mtxName << "," << l_info[0] << "," << l_info[2];
    cout << "," << l_info[4] << "," << l_info[5] << "," << l_padRatio;
    cout << "," << lastIter + 1 << "," << l_mat_partition_time << "," << l_h2d_time << "," << l_runTime << "," << (float)l_runTime / (lastIter + 1);
    cout << "," << err << endl;
    if (err == 0) {
        cout << "Test pass!" << endl;
        return EXIT_SUCCESS;
    } else {
        cout << "Test failed! There are in total " << err << " mismatches in the solution." << endl;
        return EXIT_FAILURE;
    }
}

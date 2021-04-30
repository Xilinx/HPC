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
#include "fcnKernel.hpp"
#include "utils.hpp"
#include "binFiles.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 6 || argc > 7) {
        cout << "Usage: " << argv[0]
             << " <XCLBIN File> <FCN Batch> <Output Vector Dim> <Input Vector Dim> <Data Path> [Device ID]" << endl;
        return EXIT_FAILURE;
    }

    int32_t l_index = 0;

    string binaryFile = argv[++l_index];

    int p_batch = atoi(argv[++l_index]);

    int p_m = atoi(argv[++l_index]);
    assert(p_m % HPC_numChannels == 0);
    assert(p_m % HPC_parEntries == 0);
    assert(p_batch % HPC_vecChannels == 0);

    int p_n = atoi(argv[++l_index]);
    assert(p_n % HPC_parEntries == 0);

    // I/O Data Vectors
    host_buffer_t<HPC_dataType> h_W(p_m * p_n);
    host_buffer_t<HPC_dataType> h_x(p_n * p_batch);
    host_buffer_t<HPC_dataType> h_bias(p_m);
    host_buffer_t<HPC_dataType> h_ref(p_m * p_batch);
    host_buffer_t<HPC_dataType> h_v;

    string filepath = argv[++l_index];
    readBin(filepath + "W.mat", h_W.size() * sizeof(HPC_dataType), h_W);
    readBin(filepath + "in.mat", h_x.size() * sizeof(HPC_dataType), h_x);
    readBin(filepath + "bias.mat", h_bias.size() * sizeof(HPC_dataType), h_bias);
    readBin(filepath + "out.mat", h_ref.size() * sizeof(HPC_dataType), h_ref);

    int l_deviceId = 0;
    if (argc > l_index) l_deviceId = atoi(argv[++l_index]);

    FPGA fpga(l_deviceId);
    fpga.xclbin(binaryFile);

    double t_exe =
        fcn<HPC_instrBytes, HPC_numChannels, HPC_vecChannels>(&fpga, p_batch, p_m, p_n, h_W, h_x, h_bias, h_v);
    cout << "Total execution time measured by SW is: " << t_exe << " sec." << endl;

    int err = 0;
    compare(p_m * p_batch, h_v.data(), h_ref.data(), err, true);
    if (err == 0) {
        cout << "Results verified." << endl;
        return EXIT_SUCCESS;
    } else {
        cout << "There are in total " << err << " mismatches in the solution." << endl;
        return EXIT_FAILURE;
    }
}

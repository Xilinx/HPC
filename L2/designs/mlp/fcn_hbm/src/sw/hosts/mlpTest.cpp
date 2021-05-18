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
#include "mlpKernel.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char** argv) {
    int32_t l_index = 0;

    string binaryFile = argv[++l_index];

    int p_batch = atoi(argv[++l_index]);

    int numLayers = atoi(argv[++l_index]);
    vector<uint32_t> layers;
    for (int i = 0; i < numLayers + 1; i++) layers.push_back(atoi(argv[++l_index]));

    string filePath = argv[++l_index];
    int l_numDevices = atoi(argv[++l_index]);
    host_buffer_t<HPC_dataType> h_x(layers.front() * p_batch);
    host_buffer_t<HPC_dataType> h_ref(layers.back() * p_batch);
    host_buffer_t<HPC_dataType> h_v;
    readBin(filePath + "in.mat", h_x.size() * sizeof(HPC_dataType), h_x);
    readBin(filePath + "out.mat", h_ref.size() * sizeof(HPC_dataType), h_ref);
    h_v.resize(h_ref.size());

    MLP<HPC_dataType> mlp(numLayers);
    mlp.setDim(layers.data());
    mlp.setActFunc(xf::hpc::mlp::ActFunc_t::SIGMOID);
    mlp.loadLayer(filePath.c_str());

    vector<FPGA> fpgas;
    fpgas.reserve(l_numDevices);
    for (int i = 0; i < l_numDevices; i++) {
        fpgas.emplace_back(FPGA(i));
        fpgas.back().xclbin(binaryFile);
    }

    vector<MLPKernel<HPC_dataType, HPC_instrBytes> > mlpKernels;
    mlpKernels.reserve(l_numDevices);
    for (int i = 0; i < l_numDevices; i++) {
        mlpKernels.emplace_back(
            MLPKernel<HPC_dataType, HPC_instrBytes>(HPC_numChannels, HPC_vecChannels, HPC_parEntries));
        mlpKernels.back().fpga(&fpgas[i]);
        mlpKernels.back().getCU("krnl_fcn");
        mlpKernels.back().loadModel(&mlp);
    }

    double sec = 0;
    if (l_numDevices == 1) {
        sec = mlpKernels[0].inference(h_x, h_v);
    } else {
        sec = MLPKernel<HPC_dataType, HPC_instrBytes>::inference(mlpKernels, h_x, h_v);
    }
    cout << "SW measured execution time is: " << sec << " s." << endl;

    int err = 0;
    compare(h_ref.size(), h_v.data(), h_ref.data(), err, false);
    if (err == 0) {
        cout << "Results verified." << endl;
        return EXIT_SUCCESS;
    } else {
        cout << "There are in total " << err << " mismatches in the solution." << endl;
        return EXIT_FAILURE;
    }
}

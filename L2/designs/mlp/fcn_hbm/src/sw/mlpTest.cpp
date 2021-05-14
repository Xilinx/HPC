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
#include "mlpBase.hpp"
#include "utils.hpp"

using namespace std;
using namespace xilinx_apps::mlp;

int main(int argc, char** argv) {
    int32_t l_index = 0;

    string binaryFile = argv[++l_index];

    int p_batch = atoi(argv[++l_index]);

    int numLayers = atoi(argv[++l_index]);
    vector<uint32_t> layers;
    for (int i = 0; i < numLayers + 1; i++) layers.push_back(atoi(argv[++l_index]));

    string filePath = argv[++l_index];
    int l_deviceId = 0;
    if (argc > l_index) l_deviceId = atoi(argv[++l_index]);
    host_buffer_t<HPC_dataType> h_x(layers.front() * p_batch);
    host_buffer_t<HPC_dataType> h_ref(layers.back() * p_batch);
    host_buffer_t<HPC_dataType> h_v;
    readBin(filePath + "in.mat", h_x.size() * sizeof(HPC_dataType), h_x);
    readBin(filePath + "out.mat", h_ref.size() * sizeof(HPC_dataType), h_ref);

    /*
    xf::hpc::mlp::MLP<HPC_dataType> mlp(numLayers);
    mlp.setDim(layers.data());
    mlp.setActFunc(xf::hpc::mlp::ActFunc_t::SIGMOID);
    mlp.loadLayer(filePath);
    */

    Options l_options;
    l_options.numDevices = 1;
    l_options.deviceIds[0] = l_deviceId;
    l_options.xclbinNames[0] = binaryFile;
    l_options.numCUsOnDevice[0] = 1;
    l_options.cuNames[0][0] = "krnl_fcn";

    MLPBase l_mlp(l_options);
    l_mlp.addEmptyModel(numLayers);
    l_mlp.setDim(0, layers.data());
    l_mlp.setAllActFunc(0, static_cast<uint8_t>(xf::hpc::mlp::ActFunc_t::SIGMOID));
    l_mlp.loadLayersFromFile(0, filePath.c_str());
    double sec = l_mlp.inference(h_x, h_v, 0, 0);

    /*void* mlp = createModel(numLayers);
    setDim(mlp, layers.data());
    setActFunc(mlp, static_cast<uint8_t>(xf::hpc::mlp::ActFunc_t::SIGMOID));
    loadLayer(mlp, filePath.c_str());

    FPGA fpga(l_deviceId);
    fpga.xclbin(binaryFile);
    MLPKernel<HPC_dataType, HPC_instrBytes> mlpKernel(&fpga, HPC_numChannels, HPC_vecChannels, HPC_parEntries);
    mlpKernel.getCU("krnl_fcn");
    mlpKernel.loadModel((MLP<HPC_dataType>*)mlp);
    double sec = mlpKernel.inference(h_x, h_v);*/
    cout << "SW measured execution time is: " << sec << " s." << endl;

    int err = 0;
    compare(layers.back() * p_batch, h_v.data(), h_ref.data(), err, false);
    //destroyModel(mlp);
    if (err == 0) {
        cout << "Results verified." << endl;
        return EXIT_SUCCESS;
    } else {
        cout << "There are in total " << err << " mismatches in the solution." << endl;
        return EXIT_FAILURE;
    }
}

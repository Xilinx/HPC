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
    int numDevice = atoi(argv[++l_index]);
    host_buffer_t<HPC_dataType> h_x(layers.front() * p_batch);
    host_buffer_t<HPC_dataType> h_ref(layers.back() * p_batch);
    host_buffer_t<HPC_dataType> h_v(h_ref.size());
    readBin(filePath + "in.mat", h_x.size() * sizeof(HPC_dataType), h_x);
    readBin(filePath + "out.mat", h_ref.size() * sizeof(HPC_dataType), h_ref);

    Options l_options(numDevice);
    for (int i = 0; i < numDevice; i++) l_options.deviceIds.push_back(i);
    l_options.xclbinNames = vector<string>(numDevice, binaryFile);
    l_options.numCUsOnDevice = vector<uint8_t>(numDevice, 1);
    l_options.cuNames = vector<vector<string> >(numDevice, {"krnl_fcn"});

    MLPBase l_mlp(l_options);
    l_mlp.addEmptyModel(numLayers);
    l_mlp.setDim(0, layers.data());
    l_mlp.setAllActFunc(0, "sigmoid");
    l_mlp.loadLayersFromFile(0, filePath.c_str());
    for (int i = 0; i < numDevice; i++) l_mlp.loadModel(0, i);
    double sec = l_mlp.inferenceOnAllDevices(h_x, h_v);

    cout << "SW measured execution time is: " << sec << " s." << endl;

    int err = 0;
    compare(layers.back() * p_batch, h_v.data(), h_ref.data(), err, false);
    // destroyModel(mlp);
    if (err == 0) {
        cout << "Results verified." << endl;
        return EXIT_SUCCESS;
    } else {
        cout << "There are in total " << err << " mismatches in the solution." << endl;
        return EXIT_FAILURE;
    }
}

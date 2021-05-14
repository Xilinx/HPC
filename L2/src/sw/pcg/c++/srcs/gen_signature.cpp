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

using namespace std;

int main(int argc, char** argv) {
    int arg = 0;
    string dataPath = argv[++arg];
    Signature l_sig(PAR_ENTRIES, ACC_LATENCY, CHANNELS, MAX_ROWS, MAX_COLS, MEM_BITS);
    l_sig.process(dataPath);

    string l_rbParamFileName(dataPath + "/rbParam.dat");
    l_sig.store_rbParam(l_rbParamFileName);
    string l_parParamFileName(dataPath + "/parParam.dat");
    l_sig.store_parParam(l_parParamFileName);
    string l_nnzFileNames[CHANNELS];
    for (int i = 0; i < CHANNELS; i++) {
        l_nnzFileNames[i] = dataPath + "/nnzVal_" + to_string(i) + ".dat";
    }
    l_sig.store_nnz(l_nnzFileNames);
    string l_infoFileName(dataPath + "/info.dat");
    l_sig.store_info(l_infoFileName);

    return EXIT_SUCCESS;
}

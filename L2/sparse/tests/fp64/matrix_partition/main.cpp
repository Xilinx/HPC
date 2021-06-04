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

#include "gen_signature.hpp"

using namespace std;

int main(int argc, char** argv) {
    int arg = 0;
    string dataPath = argv[++arg];
    CooMat l_mat = loadMat(dataPath);
    MatPartition l_matPar = partitionMat(l_mat);
    storeMatPar(dataPath, l_matPar);
    printf("INFO: matrix %s partiton done.\n", l_mat.m_name.c_str());
    printf("      Original m, n, nnzs = %d, %d, %d\n", l_matPar.m_m, l_matPar.m_n, l_matPar.m_nnz);
    printf("      After padding m, n, nnzs = %d, %d, %d\n", l_matPar.m_mPad, l_matPar.m_nPad, l_matPar.m_nnzPad);
    printf("      Padding overhead is %f\n", (double)(l_matPar.m_nnzPad - l_matPar.m_nnz) / l_matPar.m_nnz);
    freeMat(l_mat);
    freeMatPar(l_matPar);
    return EXIT_SUCCESS;
}

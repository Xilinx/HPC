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

/*
 * usage: ./fcn_example.exe PATH_TO_XCLBIN/gemx.xclbin PATH_TO_XCLBIN/config_info.dat
 *
 */

#include <iomanip>
#include <cmath>
#include "xf_blas.hpp"

#define IDX2R(i, j, ld) (((i) * (ld)) + (j))
#define m 128 // a - mxk matrix
#define n 128 // b - kxn matrix
#define k 128 // c - mxn matrix

using namespace std;

float* getGoldenMat(float* a, float* b, float* c) {
    float* goldenC;
    goldenC = (float*)malloc(m * n * sizeof(float));
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < n; col++) {
            float l_val = 0;
            for (int i = 0; i < k; i++) {
                l_val += a[IDX2R(row, i, k)] * b[IDX2R(i, col, n)];
            }
            goldenC[IDX2R(row, col, n)] = l_val + c[IDX2R(row, col, n)];
        }
    }
    return goldenC;
}

bool compareFCN(float* c, float* goldenC, float p_TolRel = 1e-3, float p_TolAbs = 1e-5) {
    bool l_check = true;
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < n; col++) {
            float l_ref = goldenC[IDX2R(row, col, n)];
            float l_result = c[IDX2R(row, col, n)];
            float l_diffAbs = abs(l_ref - l_result);
            float l_diffRel = l_diffAbs;
            if (goldenC[IDX2R(row, col, n)] != 0) {
                l_diffRel /= abs(l_ref);
            }
            bool check = (l_diffRel <= p_TolRel) || (l_diffAbs <= p_TolAbs);
            if (!check) {
                cout << "golden result " << setprecision(10) << goldenC[IDX2R(row, col, n)]
                     << " is not equal to fpga result " << setprecision(10) << c[IDX2R(row, col, n)] << "\n";
                l_check = false;
            }
        }
    }
    return l_check;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << " usage: \n"
             << " fcn_test.exe gemx.xclbin config_info.dat 1\n"
             << " fcn_test.exe gemx.xclbin config_info.dat\n";
        return EXIT_FAILURE;
    }
    unsigned int l_argIdx = 1;
    string l_xclbinFile(argv[l_argIdx++]);
    string l_configFile(argv[l_argIdx++]);
    int l_numKernel = 1;

    if (argc == 4) {
        cout << "read custom number of kernels\n";
        l_numKernel = stoi(argv[l_argIdx++]);
    }

    xfblasEngine_t engineName = XFBLAS_ENGINE_FCN;
    xfblasStatus_t status = xfblasCreate(l_xclbinFile.c_str(), l_configFile, engineName, l_numKernel);
    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Create Handle failed with error code: " << status << "\n";
        return EXIT_FAILURE;
    }

    int i, j; // i-row l_numKernel -1 ,j- column l_numKernel -1
    float *a, *b, *c, *bias;

    posix_memalign((void**)&a, 4096, m * k * sizeof(float));
    posix_memalign((void**)&b, 4096, k * n * sizeof(float));
    posix_memalign((void**)&c, 4096, m * n * sizeof(float));
    posix_memalign((void**)&bias, 4096, 1 * n * sizeof(float));
    memset(bias, 0, 1 * n * sizeof(float));

    int ind = 1;
    for (i = 0; i < m; i++) {
        for (j = 0; j < k; j++) {
            a[IDX2R(i, j, k)] = (float)ind++;
        }
    }
    ind = 1;
    for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
            b[IDX2R(i, j, n)] = (float)ind++;
        }
    }

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            c[IDX2R(i, j, n)] = 0;
        }
    }

    float* goldenC = getGoldenMat(a, b, c);
    
    status = xfblasMallocRestricted(m, k, sizeof(*a), a, k, l_numKernel - 1);
    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Malloc memory in device for matrix A failed with error code: " << status << "\n";
        return EXIT_FAILURE;
    }

    status = xfblasMallocRestricted(k, n, sizeof(*b), b, n, l_numKernel - 1);

    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Malloc memory in device for matrix B failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }
    status = xfblasMallocRestricted(m, n, sizeof(*c), c, n, l_numKernel - 1);

    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Malloc memory in device for matrix C failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }
    
    status = xfblasMallocRestricted(1, n, sizeof(*bias), bias, n, l_numKernel - 1);

    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Malloc memory in device for matrix bias failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }

    status = xfblasSetMatrixRestricted(a, l_numKernel - 1);
    status = xfblasSetMatrixRestricted(b, l_numKernel - 1);
    status = xfblasSetMatrixRestricted(c, l_numKernel - 1);
    status = xfblasSetMatrixRestricted(bias, l_numKernel - 1);
    
    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "sned Matrices to device failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }

    status = xfblasFcn(m, n, k, 1, a, k, b, n, 1, c, n, bias, n, 1, 0, 1, 1, l_numKernel - 1);

    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Matrix Multiplication failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }

    status = xfblasGetMatrixRestricted(c, l_numKernel - 1);

    if (status != XFBLAS_STATUS_SUCCESS) {
        cout << "Get Matrix failed with error code: " << status << "\n";
        xfblasDestroy();
        return EXIT_FAILURE;
    }

    if (compareFCN(c, goldenC)) {
        cout << "Test passed!\n";
    } else {
        cout << "Test failed!\n";
    }

    xfblasFree(a, l_numKernel - 1);
    xfblasFree(b, l_numKernel - 1);
    xfblasFree(c, l_numKernel - 1);
    xfblasFree(bias, l_numKernel - 1);
    xfblasFreeInstr(l_numKernel - 1);
    free(a);
    free(b);
    free(c);
    free(bias);
    free(goldenC);

    xfblasDestroy(l_numKernel);

    return EXIT_SUCCESS;
}
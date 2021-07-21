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
#include "pcg.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char** argv) {
    if (argc < 3) {
        printf("Usage: %s <deviceId> <xclbinPath>\n", argv[0]);
        return 1;
    }

    int deviceId = atoi(argv[1]);
    const char* xclbinPath = argv[2];

    const uint32_t p_n = 4096, p_nnz = p_n * 3 - 2;
    const double p_tol = 1e-8;
    const uint32_t p_maxIter = 1000;
    uint32_t p_iter, *p_rowIdx, *p_colIdx;
    double p_res;
    double *p_data, *matJ, *b, *x, *tmp;
    p_rowIdx = malloc(sizeof(uint32_t) * p_nnz);
    p_colIdx = malloc(sizeof(uint32_t) * p_nnz);
    p_data = malloc(sizeof(double) * p_nnz);
    matJ = malloc(sizeof(double) * p_n);
    b = malloc(sizeof(double) * p_n);
    x = malloc(sizeof(double) * p_n);
    tmp = malloc(sizeof(double) * p_n);

    uint32_t idx = 0, i = 0;

    for (i = 0; i < p_n; i++) {
        int val = rand() % p_n;
        tmp[i] = val / 73.0;
    }

    for (i = 0; i < p_n; i++) {
        if (i != 0) {
            p_rowIdx[idx] = i;
            p_colIdx[idx] = i - 1;
            p_data[idx++] = (tmp[i] + tmp[i - 1]) / 2;
        }
        if (i != p_n - 1) {
            p_rowIdx[idx] = i;
            p_colIdx[idx] = i + 1;
            p_data[idx++] = (tmp[i] + tmp[i + 1]) / 2;
        }
        p_rowIdx[idx] = i;
        p_colIdx[idx] = i;
        matJ[i] = tmp[i];
        if (i != 0) {
            matJ[i] += tmp[i - 1] * 0.75;
        }
        if (i != p_n - 1) {
            matJ[i] += tmp[i + 1] * 0.75;
        }
        p_data[idx++] = matJ[i];
    }

    for (i = 0; i < p_n; i++) {
        int val = rand() % p_n - p_n / 2;
        b[i] = val / 97.0;
    }

    void* pHandle = create_JPCG_handle(deviceId, xclbinPath);
    printf("Oha-konban-chiwa World!\n");
    JPCG_coo(pHandle, JPCG_MODE_FULL, p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ, b, x, p_maxIter, p_tol, &p_iter,
             &p_res);
    printf("Equation solved in %d iterations with relative residual %e.\n", p_iter, p_res);
    destroy_JPCG_handle(pHandle);

    free(p_rowIdx);
    free(p_colIdx);
    free(p_data);
    free(b);
    free(x);
    free(tmp);
    free(matJ);
    return 0;
}

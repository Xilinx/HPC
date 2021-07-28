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
#include <assert.h>

void genSPD(uint32_t p_n, uint32_t p_nnz, uint32_t *p_rowIdx, uint32_t *p_colIdx, double *p_data, double *matJ);

#define CheckError(code) { checkError((code), __FILE__, __LINE__); }
inline void checkError(XJPCG_Status_t code, const char *file, int line) {
    if(code != XJPCG_STATUS_SUCCESS){
        fprintf(stderr, "CheckError: %s at %s:%d\n", xJPCG_getErrorString(code), file, line);
        if(code != XJPCG_STATUS_EXECUTION_FAILED) 
            exit(code);
    }

}


int main(int argc, const char** argv) {
    if (argc < 3) {
        printf("Usage: %s <deviceId> <xclbinPath>\n", argv[0]);
        return 1;
    }

    int deviceId = atoi(argv[1]);
    const char* xclbinPath = argv[2];

    const uint32_t p_n = 65536, p_nnz = p_n * 3 - 2;
    const double p_tol = 1e-16;
    const uint32_t p_maxIter = 100;
    uint32_t p_iter, *p_rowIdx, *p_colIdx;
    double p_res;
    double *p_data, *matJ, *b, *x;
    p_rowIdx = malloc(sizeof(uint32_t) * p_nnz);
    p_colIdx = malloc(sizeof(uint32_t) * p_nnz);
    p_data = malloc(sizeof(double) * p_nnz);
    matJ = malloc(sizeof(double) * p_n);
    b = malloc(sizeof(double) * p_n);
    x = malloc(sizeof(double) * p_n);

    genSPD(p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ);

    uint32_t i = 0;
    for (i = 0; i < p_n; i++) {
        int val = rand() % p_n - p_n / 2;
        b[i] = val / 37.0;
    }

    XJPCG_Handle_t pHandle;
    CheckError(xJPCG_createHandle(&pHandle, deviceId, xclbinPath));
    CheckError(xJPCG_cooSolver(&pHandle, p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ, b, x, p_maxIter, p_tol, &p_iter,
                &p_res, XJPCG_MODE_DEFAULT));

    XJPCG_Metric_t metric;
    CheckError(xJPCG_getMetrics(&pHandle, &metric));

    printf("First equation information:\n");
    printf("\tMatrix dim:\t %d\n", p_n);
    printf("\tMatrix NNZs:\t %d\n", p_nnz);
    printf("\tNum of iterations:\t %d\n", p_iter);
    printf("\tRelative residual:\t %e\n", p_res);
    printf("\tMatrix process time:\t %fs\n", metric.m_matProc);
    printf("\tVector process time:\t %fs\n", metric.m_vecProc);
    printf("\tSolver execution time:\t %fs\n", metric.m_solver);
    printf("--------------------------------------------\n");

    genSPD(p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ);
    for (i = 0; i < p_n; i++) {
        int val = rand() % p_n - p_n / 2;
        b[i] = val / 97.0;
    }
    CheckError(xJPCG_cooSolver(&pHandle, p_n, p_nnz, NULL, NULL, p_data, matJ, b, x, p_maxIter, p_tol, &p_iter,
                &p_res, XJPCG_MODE_KEEP_NZ_LAYOUT));

    CheckError(xJPCG_getMetrics(&pHandle, &metric));

    printf("Second equation information:\n");
    printf("\tMatrix dim:\t %d\n", p_n);
    printf("\tMatrix NNZs:\t %d\n", p_nnz);
    printf("\tNum of iterations:\t %d\n", p_iter);
    printf("\tRelative residual:\t %e\n", p_res);
    printf("\tMatrix process time:\t %fs\n", metric.m_matProc);
    printf("\tVector process time:\t %fs\n", metric.m_vecProc);
    printf("\tSolver execution time:\t %fs\n", metric.m_solver);
    printf("--------------------------------------------\n");

    XJPCG_Status_t status = xJPCG_cooSolver(&pHandle, p_n, p_nnz, NULL, NULL, NULL, matJ, b, x, 10, p_tol, &p_iter,
            &p_res, XJPCG_MODE_KEEP_MATRIX);

    if(status != XJPCG_STATUS_SUCCESS) {
        const char* errMessage = xJPCG_getLastMessage(&pHandle);
        printf("Third equation status %s, error message '%s'\n", xJPCG_getErrorString(status), errMessage);
    } else {
        CheckError(xJPCG_getMetrics(&pHandle, &metric));

        printf("Third equation information:\n");
        printf("\tMatrix dim:\t %d\n", p_n);
        printf("\tMatrix NNZs:\t %d\n", p_nnz);
        printf("\tNum of iterations:\t %d\n", p_iter);
        printf("\tRelative residual:\t %e\n", p_res);
        printf("\tMatrix process time:\t %fs\n", metric.m_matProc);
        printf("\tVector process time:\t %fs\n", metric.m_vecProc);
        printf("\tSolver execution time:\t %fs\n", metric.m_solver);
        printf("--------------------------------------------\n");
    }

    CheckError(xJPCG_destroyHandle(&pHandle));

    free(p_rowIdx);
    free(p_colIdx);
    free(p_data);
    free(b);
    free(x);
    free(matJ);
    return 0;
}

void genSPD(uint32_t p_n, uint32_t p_nnz, uint32_t *p_rowIdx, uint32_t *p_colIdx, double *p_data, double *matJ){
    double *tmp = malloc(sizeof(double) * p_n);

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
    free(tmp);
    assert(p_nnz == idx);
}

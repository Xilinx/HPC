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

#ifndef GEMM_MKL_HELPER_HPP
#define GEMM_MKL_HELPER_HPP

#define IDX2R(i, j, ld) (((i) * (ld)) + (j))

#include <iostream>
#include <string>
#include <mkl.h>
#include <chrono>

#ifdef USE_DOUBLE_PRECISION
#define DISPLAY_GEMM_FUNC "DQRes_MKL"
#define XFBLAS_dataType double
#define GEMM_MKL(m, n, k, alpha, beta, a, b, c) \
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, a, k, b, n, beta, c, n);
#define GEMM_MKL_T2(m, n, k, alpha, beta, a, b, c) \
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, m, n, k, alpha, a, k, b, k, beta, c, n);
#define COPY_MKL(m, n, alpha, d, c) mkl_domatcopy('C', 'N', m, n, alpha, d, m, c, m);
#define EXP_MKL(n, a, b) vdExp(n, a, b);
#define ADD_MKL(n, a, b, y) vdAdd(n, a, b, y);
#define SUB_MKL(n, a, b, y) vdSub(n, a, b, y);
#define DIV_MKL(n, a, b, y) vdDiv(n, a, b, y);
#define MUL_MKL(n, a, b, y) vdMul(n, a, b, y);

#elif USE_SINGLE_PRECISION
#define DISPLAY_GEMM_FUNC "SQRes_MKL"
#define XFBLAS_dataType float
#define GEMM_MKL(m, n, k, alpha, beta, a, b, c) \
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, a, k, b, n, beta, c, n);
#define GEMM_MKL_T2(m, n, k, alpha, beta, a, b, c) \
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, m, n, k, alpha, a, k, b, k, beta, c, n);
#define COPY_MKL(m, n, alpha, d, c) mkl_somatcopy('C', 'N', m, n, alpha, d, m, c, m);
#define EXP_MKL(n, a, b) vsExp(n, a, b);
#define ADD_MKL(n, a, b, y) vsAdd(n, a, b, y);
#define SUB_MKL(n, a, b, y) vsSub(n, a, b, y);
#define DIV_MKL(n, a, b, y) vsDiv(n, a, b, y);
#define MUL_MKL(n, a, b, y) vsMul(n, a, b, y);

#else
#define DISPLAY_GEMM_FUNC "SQRes_MKL"
#define XFBLAS_dataType float
#define GEMM_MKL(m, n, k, alpha, beta, a, b, c) \
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, a, k, b, n, beta, c, n);
#define GEMM_MKL_T2(m, n, k, alpha, beta, a, b, c) \
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, m, n, k, alpha, a, k, b, k, beta, c, n);
#define COPY_MKL(m, n, alpha, d, c) mkl_somatcopy('C', 'N', m, n, alpha, d, m, c, m);
#define EXP_MKL(n, a, b) vsExp(n, a, b);
#define ADD_MKL(n, a, b, y) vsAdd(n, a, b, y);
#define SUB_MKL(n, a, b, y) vsSub(n, a, b, y);
#define DIV_MKL(n, a, b, y) vsDiv(n, a, b, y);
#define MUL_MKL(n, a, b, y) vsMul(n, a, b, y);

#endif

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePointType;

XFBLAS_dataType* createMat(int p_rows, int p_cols, bool is_zero, double inivalue);
void initMat(XFBLAS_dataType* mat, int p_rows, int p_cols, bool is_zero, double inivalue);
void tansig(int n, XFBLAS_dataType* a, XFBLAS_dataType* b);

XFBLAS_dataType* createMat(int p_rows, int p_cols, bool is_zero = false, double inivalue = 0.0) {
    XFBLAS_dataType* mat;
/*// OBSOLETE, use posix_memalign.
  mat = (XFBLAS_dataType *)memalign(128, (size_t)p_rows * (size_t)p_cols * sizeof(XFBLAS_dataType));
  if (mat == (XFBLAS_dataType *)NULL) {
    printf("[ERROR] failed to create the matrix\n");
    exit(1);
  }*/
/* Replacing
        void* ptr = NULL;
        int error = posix_memalign(&ptr, 16, 1024);
        if (error != 0) {

with

        void* ptr = _aligned_malloc(1024, 16);
        if (!ptr) {
*/
#ifdef WINDOWS_SYSTEM
    mat = (XFBLAS_dataType*)_aligned_malloc((size_t)p_rows * (size_t)p_cols * sizeof(XFBLAS_dataType), 4096);
    if (mat == (XFBLAS_dataType*)NULL) {
        // printf("[ERROR %d] failed to create the matrix\n", rc);
        printf("[ERROR ] failed to create the matrix\n");
        exit(1);
    }
#else
    int rc = posix_memalign((void**)&mat, 4096, (size_t)p_rows * (size_t)p_cols * sizeof(XFBLAS_dataType));
    if (rc != 0) {
        printf("[ERROR %d] failed to create the matrix\n", rc);
        exit(1);
    }
#endif
    initMat(mat, p_rows, p_cols, is_zero, inivalue);
    return mat;
}

void initMat(XFBLAS_dataType* mat, int p_rows, int p_cols, bool is_zero, double inivalue) {
    srand(time(NULL));
    for (int j = 0; j < p_rows; j++) {
        for (int i = 0; i < p_cols; i++) {
            if (is_zero)
                mat[IDX2R(j, i, p_cols)] = j * p_cols + i;
            else
                mat[IDX2R(j, i, p_cols)] = inivalue;
        }
    }
}

void sigmoid(int n, XFBLAS_dataType* a, XFBLAS_dataType* b) {
    XFBLAS_dataType *OneVector, *TwoVector, *mTwoVector;

    OneVector = createMat(n, 1, false, 1.0);
    TwoVector = createMat(n, 1, false, 2.0);
    mTwoVector = createMat(n, 1, false, -1.0);

    MUL_MKL(n, mTwoVector, a, b); // b=-a
    EXP_MKL(n, b, b);             // b=exp(-a)
    ADD_MKL(n, OneVector, b, b);  // b=1+exp(-a)
    DIV_MKL(n, OneVector, b, b);  // b=1/(1+exp(-a))

#ifdef WINDOWS_SYSTEM
    _aligned_free(OneVector);
    _aligned_free(TwoVector);
    _aligned_free(mTwoVector);
#else
    free(OneVector);
    free(TwoVector);
    free(mTwoVector);
#endif
}
void tansig(int n, XFBLAS_dataType* a, XFBLAS_dataType* b) {
    XFBLAS_dataType *OneVector, *TwoVector, *mTwoVector;

    OneVector = createMat(n, 1, false, 1.0);
    TwoVector = createMat(n, 1, false, 2.0);
    mTwoVector = createMat(n, 1, false, -2.0);

    MUL_MKL(n, mTwoVector, a, b); // b=-2*a
    EXP_MKL(n, b, b);             // b=exp(-2*a)
    ADD_MKL(n, OneVector, b, b);  // b=1+exp(-2*a)
    DIV_MKL(n, TwoVector, b, b);  // b=2/(1+exp(-2*a))
    SUB_MKL(n, b, OneVector, b);  // b=2/(1+exp(-2*a)) - 1

#ifdef WINDOWS_SYSTEM
    _aligned_free(OneVector);
    _aligned_free(TwoVector);
    _aligned_free(mTwoVector);
#else
    free(OneVector);
    free(TwoVector);
    free(mTwoVector);
#endif
}

#endif

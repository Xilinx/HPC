/* -------------------------------------------------------------------------- */
/* Basic implementation of Conjugate Gradient with a diagonal preconditioner.
 ** It assumes its being called by an AUTODOUBLE version of LS-DYNA.
 **
 ** The code is as simple as possible and should not be used for production
 ** purposes.
 **
 ** Created: F.-H. Rouet, Livermore Software Technology, LLC. Feb. 2021.
 ** Revised: R. F. Lucas, Livermore Software Technology, LLC. Apr. 2021.
 */
/* -------------------------------------------------------------------------- */

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <experimental/filesystem>
#include <assert.h>
#include <sys/time.h>
#include "utils.hpp"

#ifdef USE_FPGA
#include "pcgImp.hpp"
#include "cgHost.hpp"
typedef PCGImpl<CG_dataType,
                CG_parEntries,
                CG_instrBytes,
                SPARSE_accLatency,
                SPARSE_hbmChannels,
                SPARSE_maxRows,
                SPARSE_maxCols,
                SPARSE_hbmMemBits>
    PCG_TYPE;
#endif

using namespace std;
#ifdef AUTODOUBLE
typedef long long FortranMindex;
typedef long long FortranInteger;
typedef double FortranReal;
#else
typedef long FortranMindex;
typedef long FortranInteger;
typedef float FortranReal;
#endif

#ifdef UNSCORE
#define userLE_JPCG userle_jpcg_
#define userLE_smpv userle_smpv_
#define userLE_vecdot userle_vecdot_
#define userLE_vecsum userle_vecsum_
#endif
#ifdef UPCASE
#define userLE_JPCG USERLE_JPCG
#define userLE_spmv USERLE_SPMV
#define userLE_vecdot USERLE_VECDOR
#define userLE_vecsum USERLE_VECSUM
#endif
/* -------------------------------------------------------------------------- */
/* Prototypes */

extern "C" void userLE_JPCG(FortranInteger* handle,
                            FortranInteger* pn,
                            FortranInteger* pnz,
                            FortranInteger* colptr,
                            FortranInteger* rowind,
                            FortranReal* values,
                            FortranReal* dprec,
                            FortranInteger* pmaxit,
                            FortranReal* ptol,
                            FortranReal* b,
                            FortranReal* x,
                            FortranInteger* pniter,
                            FortranReal* pres,
                            FortranReal* pflops);

void userLE_spmv(FortranInteger n,
                 FortranInteger nz,
                 FortranInteger* colptr,
                 FortranInteger* rowind,
                 FortranReal* values,
                 FortranReal* x,
                 FortranReal* y);

FortranReal userLE_vecdot(FortranInteger n, FortranReal* x, FortranReal* y);

void userLE_vecsum(FortranInteger n, FortranReal alpha, FortranReal* x, FortranReal beta, FortranReal* y);

template <typename FInt, typename Integer, typename Real>
void getCOO(FInt n, FInt nz, FInt* colptr, FInt* rowind, Real* values, Real* matA, Integer* rowInd, Integer* colInd);
#ifdef USE_FPGA
double fpga_JPCG(FortranInteger* handle,
                 FortranInteger pn,
                 FortranInteger pnz,
                 FortranReal* matJ,
                 FortranInteger pmaxit,
                 FortranReal ptol,
                 FortranReal* b,
                 FortranReal* x,
                 FortranInteger* pniter,
                 FortranReal* prelres,
                 FortranReal* pflops) {
    PCG_TYPE& l_pcg = *(PCG_TYPE*)(*handle);
    double t_sec;
    std::cout << "running fpga_JPCG..." << std::endl;
    TimePointType l_timer[3];
    l_timer[0] = std::chrono::high_resolution_clock::now();
    l_pcg.setVec(pn, b, matJ);
    showTimeData("Vector initialization & transmission time: ", l_timer[0], l_timer[1]);
    Results<CG_dataType> l_res = l_pcg.run(pmaxit, ptol);
    showTimeData("PCG run time: ", l_timer[2], l_timer[3], &t_sec);
    *prelres = sqrt(l_res.m_residual / l_pcg.getDot());
    *pniter = l_res.m_nIters;
    memcpy(reinterpret_cast<uint8_t*>(x), reinterpret_cast<uint8_t*>(l_res.m_x), pn * sizeof(FortranReal));
    *pflops = l_res.m_nIters * (2 * pnz + 16 * pn) - 2 * pn;
    return t_sec * 1e3;
}
#endif

extern "C" {
void userle_jpcg_create_handle_(FortranInteger* handle, 
        FortranInteger * device_id,
        const char * xclbinPath) {
#ifdef USE_FPGA
    *handle = (FortranInteger) new PCG_TYPE(*device_id, xclbinPath);
#endif
}

void userle_jpcg_set_matrix_(FortranInteger* handle,
                            FortranInteger* pn,
                            FortranInteger* pnz,
                            FortranInteger* colptr,
                            FortranInteger* rowind,
                            FortranReal* values) {
#ifdef USE_FPGA
    cout << "Setting and partitioning matrix ... " << endl;
    PCG_TYPE& l_pcg = *(PCG_TYPE*)(*handle);
    FortranInteger n = *pn;
    FortranInteger nz = *pnz;
    FortranInteger nnz = nz * 2 - n;
    FortranReal* matA;
    uint32_t* rowInd;
    uint32_t* colInd;
    matA = (FortranReal*)malloc(nnz * sizeof(FortranReal));
    rowInd = (uint32_t*)malloc(nnz * sizeof(uint32_t));
    colInd = (uint32_t*)malloc(nnz * sizeof(uint32_t));
    getCOO(n, nnz, colptr, rowind, values, matA, rowInd, colInd);
    l_pcg.setCooMat(n, nnz, rowInd, colInd, matA);
    free(matA);
    free(rowInd);
    free(colInd);
#endif
}

/* -------------------------------------------------------------------------- */
/* Conjugate Gradients with diagonal preconditioner */
void userLE_JPCG(FortranInteger* handle,
                 FortranInteger* pn,
                 FortranInteger* pnz,
                 FortranInteger* colptr,
                 FortranInteger* rowind,
                 FortranReal* values,
                 FortranReal* dprec,
                 FortranInteger* pmaxit,
                 FortranReal* ptol,
                 FortranReal* b,
                 FortranReal* x,
                 FortranInteger* pniter,
                 FortranReal* prelres,
                 FortranReal* pflops) {
    FortranInteger n, nz, maxit;
    FortranInteger niter;
    FortranInteger i;
    FortranReal tol, rnrm0;
    FortranReal rTr, rTz;
    FortranReal alpha, beta;
    FortranReal flops = 0;

    /* Fortran passed these parameters by rference */
    n = *pn;
    nz = *pnz;
    maxit = *pmaxit;
    tol = *ptol;
    FortranInteger nnz = nz * 2 - n;
    TimePointType l_start = std::chrono::high_resolution_clock::now();

#ifdef USE_FPGA
    double l_hwTime = fpga_JPCG(handle, n, nnz, dprec, maxit, tol, b, x, pniter, prelres, pflops);
#else
    FortranReal *r, *z, *p, *q;
    /* Allocate local storage */
    r = (FortranReal*)malloc(n * sizeof(FortranReal));
    z = (FortranReal*)malloc(n * sizeof(FortranReal));
    p = (FortranReal*)malloc(n * sizeof(FortranReal));
    q = (FortranReal*)malloc(n * sizeof(FortranReal));
    /* Initial solution is zero, residual is the RHS */
    for (i = 0; i < n; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    /* Norm of the initial residual */
    rTr = userLE_vecdot(n, r, r);
    rnrm0 = sqrt(rTr);
    flops += 2.0 * n;

    /* Initial preconditioned residual */
    for (i = 0; i < n; i++) z[i] = r[i] / dprec[i];
    flops += 1.0 * n;

    /* First search direction */
    for (i = 0; i < n; i++) p[i] = z[i];

    /* Main loop */
    for (niter = 1; niter <= maxit; niter++) {
/* q = A * p */
        userLE_spmv(n, nz, colptr, rowind, values, p, q);
        flops += 4.0 * nz - 2.0 * n;
        
        /* alpha = ( r^T z ) / ( p^T q ) */
        rTz = userLE_vecdot(n, r, z);
        alpha = rTz / userLE_vecdot(n, p, q);
        flops += 4.0 * n;

        /* x <- x + alpha * p */
        userLE_vecsum(n, alpha, p, 1.0, x);
        flops += 3.0 * n;

        /* r <- r - alpha * q */
        userLE_vecsum(n, -alpha, q, 1.0, r);
        flops += 3.0 * n;

        /* Compute the norm of the residual */
        rTr = userLE_vecdot(n, r, r);
        flops += 2.0 * n;

        /* Stop if residual is small enough */
        if (sqrt(rTr) / rnrm0 < tol) break;

        /* New preconditioned residual */
        for (i = 0; i < n; i++) z[i] = r[i] / dprec[i];
        flops += 1.0 * n;

        /* beta = ( r^T z ) / ( old r^T z ) */
        beta = userLE_vecdot(n, r, z) / rTz;
        flops += 2.0 * n;

        /* New search direction */
        userLE_vecsum(n, 1.0, z, beta, p);
        flops += 3.0 * n;
    }

    /* Clean up */
    free(r);
    free(z);
    free(p);
    free(q);

    /* Done */
    *pniter = niter;
    *prelres = sqrt(rTr) / rnrm0;
    *pflops = flops;
#endif


    auto l_stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = l_stop - l_start;
    double l_timeMs = l_durationSec.count() * 1e3;

    string filename = "benchmark.csv";
    bool header = std::experimental::filesystem::exists(filename);

    fstream fs;
    fs.open("benchmark.csv", ios::out | ios::app);
    if (!header) {
        fs << "Dim"
           << ","
           << "NNZ"
           << ","
           << "Niter"
           << ","
           << "Relres"
           << ","
           << "MFLOP"
           << ","
#ifdef USE_FPGA
           << "PCG Time [ms]"
           << ","
           << "PCG GFLOPS"
           << ","
#endif
           << "PCG&Matrix Partition Time [ms]"
           << ","
           << "PCG&Matrix Partition GFLOPS" << endl;
    }

    fs << n << "," << nnz << "," << *pniter << "," << *prelres << "," << *pflops / 1e6 << ","
#ifdef USE_FPGA
       << l_hwTime << "," << *pflops / 1e6 / l_hwTime << ","
#endif
       << l_timeMs << "," << *pflops / 1e6 / l_timeMs << endl;
    fs.close();
    return;
}
}

template <typename FInt, typename Integer, typename Real>
void getCOO(FInt n, FInt nz, FInt* colptr, FInt* rowind, Real* values, Real* matA, Integer* rowInd, Integer* colInd) {
    Integer index = 0;
    for (Integer j = 0; j < n; j++) {
        for (Integer k = colptr[j] - 1; k < colptr[j + 1] - 1; k++) {
            Integer i = rowind[k] - 1;
            assert(index < nz);
            rowInd[index] = i;
            colInd[index] = j;
            matA[index++] = values[k];
            if (i != j) {
                assert(index < nz);
                rowInd[index] = j;
                colInd[index] = i;
                matA[index++] = values[k];
            }
        }
    }
    assert(index == nz);
}

/* -------------------------------------------------------------------------- */
/* Sparse matrix-vector multiply, y <- beta * y + alpha * A * x  */
void userLE_spmv(FortranInteger n,
                 FortranInteger nz,
                 FortranInteger* colptr,
                 FortranInteger* rowind,
                 FortranReal* values,
                 FortranReal* x,
                 FortranReal* y) {
    FortranInteger i, j, k;

    for (i = 0; i < n; i++) y[i] = 0;

    for (j = 0; j < n; j++) {
        for (k = colptr[j] - 1; k < colptr[j + 1] - 1; k++) {
            i = rowind[k] - 1;
            y[i] += values[k] * x[j];
            if (i != j) {
                y[j] += values[k] * x[i];
            }
        }
    }

    return;
}

/* -------------------------------------------------------------------------- */
/* Vector dot product */
FortranReal userLE_vecdot(FortranInteger n, FortranReal* x, FortranReal* y) {
    FortranInteger i;
    FortranReal dot;

    dot = 0.0;
    for (i = 0; i < n; i++) dot += x[i] * y[i];

    return dot;
}

/* -------------------------------------------------------------------------- */
/* y <- beta * y + alpha * x */
void userLE_vecsum(FortranInteger n, FortranReal alpha, FortranReal* x, FortranReal beta, FortranReal* y) {
    FortranInteger i;

    for (i = 0; i < n; i++) y[i] = beta * y[i] + alpha * x[i];

    return;
}
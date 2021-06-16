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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>

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
extern "C" void userLE_JPCG(FortranInteger* pn,
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
                 FortranReal alpha,
                 FortranReal* x,
                 FortranReal beta,
                 FortranReal* y);

FortranReal userLE_vecdot(FortranInteger n, FortranReal* x, FortranReal* y);

void userLE_vecsum(FortranInteger n, FortranReal alpha, FortranReal* x, FortranReal beta, FortranReal* y);

template<typename Integer, typename Real>
void coo_spmv(Integer n,
              Integer nz,
              Integer* rowInd,
              Integer* colInd,
              Real* mat,
              Real alpha,
              Real* x,
              Real beta,
              Real* y);

template<typename Integer, typename Real>
void getCOO(Integer n,
            Integer nz,
            Integer* colptr,
            Integer* rowind,
            Real* values,
            Real* matJ,
            Real* matA,
            Integer* rowInd,
            Integer* colInd);

extern "C" void fpga_JPCG(FortranInteger pn,
                 FortranInteger pnz,
                 FortranInteger* rowInd,
                 FortranInteger* colInd,
                 FortranReal* matA,
                 FortranReal* matJ,
                 FortranInteger pmaxit,
                 FortranReal* ptol,
                 FortranReal* b,
                 FortranReal* x,
                 FortranInteger* pniter,
                 FortranReal* prelres,
                 FortranReal* pflops);
/* -------------------------------------------------------------------------- */
/* Conjugate Gradients with diagonal preconditioner */
extern "C" void userLE_JPCG(FortranInteger* pn,
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
    FortranReal *r, *z, *p, *q;

    /* Fortran passed these parameters by rference */
    n = *pn;
    nz = *pnz;
    maxit = *pmaxit;
    tol = *ptol;

    /* Allocate local storage */
    r = (FortranReal*) malloc(n * sizeof(FortranReal));
    z = (FortranReal*) malloc(n * sizeof(FortranReal));
    p = (FortranReal*) malloc(n * sizeof(FortranReal));
    q = (FortranReal*) malloc(n * sizeof(FortranReal));

#if defined FORMAT_COO || defined FPGA
    FortranInteger nnz = nz * 2 - n;
    FortranReal* matA, *matJ;
    FortranInteger* rowInd;
    FortranInteger* colInd;
    matJ = (FortranReal*) malloc(n * sizeof(FortranReal));
    matA = (FortranReal*) malloc(nnz * sizeof(FortranReal));
    rowInd = (FortranInteger*) malloc(nnz * sizeof(FortranInteger));
    colInd = (FortranInteger*) malloc(nnz * sizeof(FortranInteger));
    getCOO(n, nnz, colptr, rowind, values, matJ, matA, rowInd, colInd);
#endif

#ifdef FPGA
    fpga_JPCG(n, nnz, rowInd, colInd, matA, matJ, maxit, ptol, b, x, pniter, prelres, pflops);
#else
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
#ifdef FORMAT_COO
    for (i = 0; i < n; i++) z[i] = r[i] * matJ[i];
#else
    for (i = 0; i < n; i++) z[i] = r[i] / dprec[i];
#endif
    flops += 1.0 * n;

    /* First search direction */
    for (i = 0; i < n; i++) p[i] = z[i];

    /* Main loop */
    for (niter = 1; niter <= maxit; niter++) {
/* q = A * p */
#ifdef FORMAT_COO
        coo_spmv(n, nnz, rowInd, colInd, matA, 1.0, p, 0.0, q);
#else
        userLE_spmv(n, nz, colptr, rowind, values, 1.0, p, 0.0, q);
#endif
        flops += 6.0 * nz - 2.0 * n;

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

        /* Stop if residual is small enough */
        if (sqrt(rTr) / rnrm0 < tol) break;

        /* New preconditioned residual */
#ifdef FORMAT_COO
    for (i = 0; i < n; i++) z[i] = r[i] * matJ[i];
#else
    for (i = 0; i < n; i++) z[i] = r[i] / dprec[i];
#endif
        flops += 1.0 * n;

        /* beta = ( r^T z ) / ( old r^T z ) */
        beta = userLE_vecdot(n, r, z) / rTz;
        flops += 3.0 * n;

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

#if defined FORMAT_COO || defined FPGA
    free(matA);
    free(matJ);
    free(rowInd);
    free(colInd);
#endif

    return;
}

template<typename Integer, typename Real>
void getCOO(Integer n,
            Integer nz,
            Integer* colptr,
            Integer* rowind,
            Real* values,
            Real* matJ,
            Real* matA,
            Integer* rowInd,
            Integer* colInd) {
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
            } else {
                matJ[i] = 1.0 / values[k];
            }
        }
    }
    assert(index == nz);
}

template<typename Integer, typename Real>
void coo_spmv(Integer n,
              Integer nz,
              Integer* rowInd,
              Integer* colInd,
              Real* mat,
              Real alpha,
              Real* x,
              Real beta,
              Real* y) {
    for (Integer i = 0; i < n; i++) y[i] *= beta;

    for (Integer i = 0; i < nz; i++) {
        Integer row = rowInd[i], col = colInd[i];
        assert(row < n && col < n);
        y[row] += alpha * mat[i] * x[col];
    }

    return;
}

/* -------------------------------------------------------------------------- */
/* Sparse matrix-vector multiply, y <- beta * y + alpha * A * x  */
void userLE_spmv(FortranInteger n,
                 FortranInteger nz,
                 FortranInteger* colptr,
                 FortranInteger* rowind,
                 FortranReal* values,
                 FortranReal alpha,
                 FortranReal* x,
                 FortranReal beta,
                 FortranReal* y) {
    FortranInteger i, j, k;

    for (i = 0; i < n; i++) y[i] *= beta;

    for (j = 0; j < n; j++) {
        for (k = colptr[j] - 1; k < colptr[j + 1] - 1; k++) {
            i = rowind[k] - 1;
            y[i] += alpha * values[k] * x[j];
            if (i != j) {
                y[j] += alpha * values[k] * x[i];
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

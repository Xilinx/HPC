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
/**
 * NOT FOR CHECK-IN!
 */

#ifndef PCG_H
#define PCG_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Define this macro to make functions in pcg_loader.cpp inline instead of extern.  You would use this macro
 * when including pcg_loader.cpp in a header file, as opposed to linking with libXilinxPcg_loader.a.
 */
#ifdef XILINX_PCG_INLINE
#define XILINX_PCG_LINKAGE_DECL inline
#else
#define XILINX_PCG_LINKAGE_DECL extern
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * List of XJPCG API status
 *
 */
typedef enum {
    XJPCG_STATUS_SUCCESS,           // success status
    XJPCG_STATUS_NOT_INITIALIZED,   // handle not initialized
    XJPCG_STATUS_ALLOC_FAILED,      // FPGA memory allocation failure
    XJPCG_STATUS_INVALID_VALUE,     // invalid parameters
    XJPCG_STATUS_EXECUTION_FAILED,  // solver not convergent
    XJPCG_STATUS_INTERNAL_ERROR,    // internal errors or bugs
    XJPCG_STATUS_NOT_SUPPORTED      // unsupported behavior
} XJPCG_Status_t;

/** xJPCG_getErrorString get the string presentation of given status code
 *
 * code status code
 * return string
 */
const char* xJPCG_getErrorString(const XJPCG_Status_t code);

/**
 * Define XJPCG metrics 
 *
 */
typedef struct {
    double m_init;          // JPCG Object initialization time
    double m_matProc;       // Matrix processing time
    double m_vecProc;       // Vector processing time
    double m_solver;        // Solver execution time
} XJPCG_Metric_t;

/**
 * Define XJPCG handle type 
 *
 */
typedef struct {
    void *pcg;              // pointer to pcg object
    void *status;
    int code;               // checking code
} XJPCG_Handle_t;

/**
 * List of XJPCG solver modes
 *
 */
typedef enum XJPCG_Mode { 
    XJPCG_MODE_DEFAULT = 0x00, // Used for completely new data
    XJPCG_MODE_KEEP_NZ_LAYOUT = 0x01, // Update matrix values only
    XJPCG_MODE_KEEP_MATRIX = 0x03 // Reuse last matrix
} XJPCG_Mode;

/** xJPCG_createHandle create a JPCG handle
 *
 * deviceId the ID of the device used for JPCG solver
 * xclbinPath the path to kernel xclbin
 *
 * return the pointer of created handle
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_createHandle(XJPCG_Handle_t *handle, const int deviceId, const char* xclbinPath);

/** xJPCG_destroyHandle destroy given JPCG handle
 *
 * handel pointer to the JPCG handle to be destroyed
 *
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_destroyHandle(XJPCG_Handle_t *handle);

/** xJPCG_cooSolver solves equation Ax = b with sparse matrix A in COO format
 *
 * handle pointer to a JPCG handle
 * mode solver modes
 * p_n dimension of given matrix and vectors
 * p_nnz number of non-zero entris in sparse matrix A
 * p_rowIdx row index of matrix A
 * p_colIdx col index of matrix A
 * p_data data entries of matrix A
 * p_diagA diagnal vector of matrix A
 * p_b right-hand side vector
 * p_x solution to the equation
 * p_maxIter maximum number of iteration that solve could run
 * p_tol the relative tolerence for solver to stop iteration
 * p_iter the real iterations that solver takes
 * p_res the relative residual when solver exits
 *
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_cooSolver(XJPCG_Handle_t *handle,
        const uint32_t p_n,
        const uint32_t p_nnz,
        const uint32_t* p_rowIdx,
        const uint32_t* p_colIdx,
        const double* p_data,
        const double* p_diagA,
        const double* p_b,
        const double* p_x,
        const uint32_t p_maxIter,
        const double p_tol,
        uint32_t* p_iter,
        double* p_res,
        const XJPCG_Mode mode);

/** xJPCG_peekAtLastStatus get the last status associated with handle
 *
 * handle pointer to a JPCG handle
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Handle_t *handle);


/** xJPCG_getLastMessage get the last status/error message associated with handle
 *
 * handle pointer to a JPCG handle
 */
XILINX_PCG_LINKAGE_DECL
const char* xJPCG_getLastMessage(const XJPCG_Handle_t *handle);

/** xJPCG_getMetrics get the last performance metrics associated with handle
 *
 * handle pointer to a JPCG handle
 * metric pointer to a metric struct
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_getMetrics(XJPCG_Handle_t *handle, XJPCG_Metric_t *metric);

#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

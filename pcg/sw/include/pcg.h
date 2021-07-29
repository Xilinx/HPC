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

#ifndef XILINX_PCG_H
#define XILINX_PCG_H

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
    XJPCG_STATUS_DYNAMIC_LOADING_ERROR  // error while trying to access the API via dynamic loading (dlopen)
} XJPCG_Status_t;

/** xJPCG_getErrorString get the string presentation of given status code
 *
 * code status code
 * return string
 */
XILINX_PCG_LINKAGE_DECL
const char* xJPCG_getErrorString(const XJPCG_Status_t code);

/**
 * Define XJPCG metrics 
 *
 */
typedef struct {
    double m_objInit;    // JPCG Object initialization time
    double m_matProc; // Matrix processing time
    double m_vecProc; // Vector processing time
    double m_solver;  // Solver execution time
} XJPCG_Metric_t;


struct XJPCG_ObjectStruct; // dummy struct for XJPCG object type safety

/**
 * Opaque struct for holding the state of JPCG operations
 */
typedef struct XJPCG_ObjectStruct XJPCG_Object_t;

/**
 * List of XJPCG solver modes
 */
typedef enum XJPCG_Mode {
    XJPCG_MODE_DEFAULT = 0x00,        // Used for completely new data
    XJPCG_MODE_KEEP_NZ_LAYOUT = 0x01, // Update matrix values only
    XJPCG_MODE_KEEP_MATRIX = 0x02     // Reuse last matrix
} XJPCG_Mode;

/** xJPCG_createHandle create a JPCG handle
 *
 * handle a pointer to the JPCG handle variable that will receive the PCG handle
 * deviceId the ID of the device used for JPCG solver
 * xclbinPath the path to kernel xclbin
 *
 * return API status
 * 
 * If the initialization fails, `*handle` may remain unchanged from its original value.
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_createHandle(XJPCG_Object_t **handle, const int deviceId, const char* xclbinPath);

/** xJPCG_destroyHandle destroy given JPCG handle
 *
 * handel JPCG handle to be destroyed
 *
 * return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_destroyHandle(XJPCG_Object_t *handle);

/** xJPCG_cscSymSolver solves equation Ax = b with sparse SPD matrix A in CSC format
 *
 * handle pointer to a JPCG handle
 * mode solver modes
 * p_n dimension of given matrix and vectors
 * p_nnz number of non-zero entris in sparse matrix A
 * p_rowIdx row index of matrix A
 * p_colptr compressed col index of matrix A
 * p_data data entries of matrix A, half matrix for symmetry
 * p_diagA diagnal vector of matrix A
 * p_b right-hand side vector
 * p_x solution to the equation
 * p_maxIter maximum number of iteration that solve could run
 * p_tol the relative tolerence for solver to stop iteration
 * p_iter the real iterations that solver takes
 * p_res the relative residual when solver exits
 *
 * return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_cscSymSolver(XJPCG_Object_t *handle,
                               const uint32_t p_n,
                               const uint32_t p_nnz,
                               const uint64_t* p_rowIdx,
                               const uint64_t* p_colptr,
                               const double* p_data,
                               const double* p_diagA,
                               const double* p_b,
                               const double* p_x,
                               const uint32_t p_maxIter,
                               const double p_tol,
                               uint32_t* p_iter,
                               double* p_res,
                               const XJPCG_Mode mode);
/** xJPCG_cooSolver solves equation Ax = b with sparse SPD matrix A in COO format
 *
 * handle pointer to a JPCG handle
 * mode solver modes
 * p_n dimension of given matrix and vectors
 * p_nnz number of non-zero entris in sparse matrix A
 * p_rowIdx row index of matrix A
 * p_colIdx col index of matrix A
 * p_data data entries of matrix A, full matrix
 * p_diagA diagnal vector of matrix A
 * p_b right-hand side vector
 * p_x solution to the equation
 * p_maxIter maximum number of iteration that solve could run
 * p_tol the relative tolerence for solver to stop iteration
 * p_iter the real iterations that solver takes
 * p_res the relative residual when solver exits
 *
 * return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_cooSolver(XJPCG_Object_t *handle,
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
 * handle JPCG handle
 *
 * return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Object_t *handle);

/** xJPCG_getLastMessage get the last status/error message associated with handle
 *
 * handle pointer to a JPCG handle
 *
 * return last status message
 * 
 * NOTE: This function requires @ref xJPCG_createHandle() to have progressed sufficiently far to produce
 * a valid `handle`, except in the case of a dynamic loading error, for which even a null handle will
 * produce a string for the cause of the loading error.
 * 
 * Note also that while dynamic loading operations themselves are thread safe, because there is only one
 * global storage for the loading error, fetching the error message is not thread safe.
 */
XILINX_PCG_LINKAGE_DECL
const char* xJPCG_getLastMessage(const XJPCG_Object_t *handle);

/** xJPCG_getMetrics get the last performance metrics associated with handle
 *
 * handle JPCG handle
 * metric pointer to a metric struct
 *
 * return API status
 * 
 * This function fills the members of the given struct with performance metrics.
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_getMetrics(const XJPCG_Object_t *handle, XJPCG_Metric_t* metric);

#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

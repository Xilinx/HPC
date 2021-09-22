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
 * @brief Define a list of XJPCG API return status
 */
typedef enum {
    XJPCG_STATUS_SUCCESS,              /// success status
    XJPCG_STATUS_NOT_INITIALIZED,      /// handle not initialized
    XJPCG_STATUS_ALLOC_FAILED,         /// FPGA memory allocation failure
    XJPCG_STATUS_INVALID_VALUE,        /// invalid parameters
    XJPCG_STATUS_EXECUTION_FAILED,     /// solver not convergent
    XJPCG_STATUS_INTERNAL_ERROR,       /// internal errors or bugs
    XJPCG_STATUS_DYNAMIC_LOADING_ERROR /// error while trying to access the API via dynamic loading (dlopen)
} XJPCG_Status_t;

/**
 * @brief xJPCG_getErrorString get the string presentation of a given status code
 * @param code status code
 * @return error message string
 */
XILINX_PCG_LINKAGE_DECL
const char* xJPCG_getErrorString(const XJPCG_Status_t code);

/**
 * @brief Define XJPCG metrics
 */
typedef struct {
    double m_objInit; /// JPCG handle creation time
    double m_matProc; /// Matrix processing time
    double m_vecProc; /// Vector processing time
    double m_solver;  /// Solver execution time
} XJPCG_Metric_t;

struct XJPCG_ObjectStruct; /// dummy struct for XJPCG object type safety

/**
 * @brief Opaque struct for holding the state of JPCG operations
 */
typedef struct XJPCG_ObjectStruct XJPCG_Handle_t;

/**
 * @brief List of XJPCG solver modes
 */
typedef enum XJPCG_Mode_t {
    XJPCG_MODE_DEFAULT = 0x00,        /// Used for completely new data
    XJPCG_MODE_KEEP_NZ_LAYOUT = 0x01, /// Update matrix values only
    XJPCG_MODE_KEEP_MATRIX = 0x02,    /// Reuse last matrix
    XJPCG_MODE_C_INDEX = 0x00,        /// Default C-Type index, starting from 0
    XJPCG_MODE_FORTRAN_INDEX = 0x10   /// Fortran-Type index, starting from 1
} XJPCG_Mode_t;

/**
 * @brief xJPCG_createHandle create a JPCG handle
 * @param handle a pointer to the JPCG handle variable that will receive the PCG handle
 * @param xclbinPath the path to xclbin file
 * @return API status
 * If the initialization fails, `*handle` may remain unchanged from its original value.
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_createHandle(XJPCG_Handle_t** handle, const char* xclbinPath);

/** @brief xJPCG_destroyHandle destroies a given JPCG handle
 *
 * @param handel JPCG handle to be destroyed
 *
 * @return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_destroyHandle(XJPCG_Handle_t* handle);

/** @brief xJPCG_cscSymSolver solves a system Ax = b with sparse SPD (Symmetric Positive Definite) matrix A in CSC
 * format
 *
 * Convergency issue:
 *
 * conjugate gradient method may suffer convergency issue for matrices with large condition number.
 * Jacobi preconditioner adopted in this product is used to address this issue. However, for some matrices,
 * even with Jacobi preconditioner the solver is not able to converge. If the returned value in parameter `p_iter`
 * is equal to the maximum iteration numbers given by parameter `p_maxIter`, that means the solver cannot
 * converge.
 *
 * Mismatch issue:
 *
 * for some matrices, the solver can terminate within the given number of iterations set by parameter
 * `p_maxIter`, but produce the solution x that have some mismatches compared to the golden reference. This issue can
 * be addressed by further reducing the relative tolerance value set by parameter `p_tol`. For example, using `p_tol =
 * 1e-15`
 * might get the solver to produce correct results.
 *
 * @param handle pointer to a JPCG handle
 * @param p_n dimension of given matrix and vectors
 * @param p_nnz number of none-zero entries in p_data of sparse matrix A
 *
 *     When using this API, only upper or lower triangular part and the main diagonal entries of the matrix A are
 * stored.
 *     Therefore, the value of `p_nnz` should be equal to the (total_nnz + p_n) / 2, where total_nnz is the total number
 *     of none-zero entries in A, and the main diagonal entries in A are all none-zero entries. Users might need to pad
 *     the main diagoal entries when using this API.
 *
 * @param p_rowIdx row index of matrix A
 *
 *     when parameter `mode` contains value `XJPCG_MODE_FORTRAN_INDEX`, the row indices start from 1.
 *     when parameter `mode` contains value `XJPCG_MODE_C_INDEX`, the row indices start from 0.
 *
 * @param p_colptr compressed col index pointer of matrix A
 *
 *     when parameter `mode` contains value `XJPCG_MODE_FORTRAN_INDEX`, the col index pointer value starts from 1.
 *     when parameter `mode` contains value `XJPCG_MODE_C_INDEX`, the col index pointer value  starts from 0.
 *
 * @param p_data data entries of matrix A, half matrix for symmetry
 * @param p_diagA diagnal vector of matrix A
 * @param p_b right-hand side vector
 * @param p_x solution to the equation
 * @param p_maxIter maximum number of iteration that solver could run
 * @param p_tol the relative tolerence for solver to stop iteration
 * @param p_iter the real iterations that solver takes
 * @param p_res the relative residual when solver exits
 * @param mode solver modes including date reuse and index type
 *
 *     when using this API, the value of parameter `mode` normally contains two parts.
 *     One part is used for matrix reusage, the other for selecting C/Fortran storage.
 *     For example, setting `mode` with `XJPCG_MODE_DEFAULT | XJPCG_MODE_FORTRAN_INDEX`
 *     means the matrix A will not be re-used to solve this system, and the matrix storage follows
 *     the convention in Fortran, namely indices start from 1.
 *
 * @return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_cscSymSolver(XJPCG_Handle_t* handle,
                                  const int64_t p_n,
                                  const int64_t p_nnz,
                                  const int64_t* p_rowIdx,
                                  const int64_t* p_colptr,
                                  const double* p_data,
                                  const double* p_diagA,
                                  const double* p_b,
                                  const double* p_x,
                                  const int64_t p_maxIter,
                                  const double p_tol,
                                  int64_t* p_iter,
                                  double* p_res,
                                  const XJPCG_Mode_t mode);
/** @brief xJPCG_cooSolver solves equation Ax = b with sparse SPD matrix A in COO format
 *
 * This function only supports C storage format. Hence the indices values start from 0.
 *
 * @see xJPCG_cscSymSolver for the convergency issues and solutions to the mismatching results when using this API
 *
 * @param handle pointer to a JPCG handle
 * @param p_n dimension of given matrix and vectors
 * @param p_nnz number of non-zero entris in sparse matrix A
 * @param p_rowIdx row index of matrix A
 * @param p_colIdx col index of matrix A
 * @param p_data data entries of matrix A, full matrix
 * @param p_diagA diagnal vector of matrix A
 * @param p_b right-hand side vector
 * @param p_x solution to the equation
 * @param p_maxIter maximum number of iteration that solve could run
 * @param p_tol the relative tolerence for solver to stop iteration
 * @param p_iter the real iterations that solver takes
 * @param p_res the relative residual when solver exits
 * @param mode solver modes including date reuse and index type
 *
 * @return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_cooSolver(XJPCG_Handle_t* handle,
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
                               const XJPCG_Mode_t mode);

/** @brief xJPCG_peekAtLastStatus get the last status associated with handle
 *
 * @param handle JPCG handle
 *
 * @return API status
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Handle_t* handle);

/** @brief xJPCG_getLastMessage get the last status/error message associated with handle
 *
 * NOTE: This function requires @ref xJPCG_createHandle() to have progressed sufficiently far to produce
 * a valid `handle`, except in the case of a dynamic loading error, for which even a null handle will
 * produce a string for the cause of the loading error.
 *
 * Note also that while dynamic loading operations themselves are thread safe, because there is only one
 * global storage for the loading error, fetching the error message is not thread safe.
 *
 * @param handle pointer to a JPCG handle
 *
 * @return last status message
 *
 */
XILINX_PCG_LINKAGE_DECL
const char* xJPCG_getLastMessage(const XJPCG_Handle_t* handle);

/** @brief xJPCG_getMetrics get the last performance metrics associated with handle
 *
 * This function fills the members of the given struct with performance metrics.
 *
 * @param handle JPCG handle
 * @param metric pointer to a metric struct
 *
 * @return API status
 *
 */
XILINX_PCG_LINKAGE_DECL
XJPCG_Status_t xJPCG_getMetrics(const XJPCG_Handle_t* handle, XJPCG_Metric_t* metric);

#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

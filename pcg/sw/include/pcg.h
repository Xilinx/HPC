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

/**
 * Define this macro to make functions in pcg_loader.cpp inline instead of extern.  You would use this macro
 * when including pcg_loader.cpp in a header file, as opposed to linking with libXilinxCosineSim_loader.a.
 */
#ifdef XILINX_PCG_INLINE
#define XILINX_PCG_LINKAGE_DECL inline
#else
#define XILINX_PCG_LINKAGE_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JPCG_Mode { JPCG_MODE_DEFAULT = 0x00, // Used for completely new data
    JPCG_MODE_KEEP_NZ_LAYOUT = 0x01, // Update matrix values only
    JPCG_MODE_KEEP_MATRIX = 0x03 // Reuse last matrix
} JPCG_Mode;

/** create_JPCG_handle create a JPCG handle
 *
 * deviceId the ID of the device used for JPCG solver
 * xclbinPath the path to kernel xclbin
 *
 * return the pointer of created handle
 */
XILINX_PCG_LINKAGE_DECL
void* create_JPCG_handle(const int deviceId, const char* xclbinPath);

/** destroy_JPCG_handle destroy given JPCG handle
 *
 * handel pointer to the JPCG handle to be destroyed
 *
 */
XILINX_PCG_LINKAGE_DECL
void destroy_JPCG_handle(void* handle);

/** JPCG_coo solve equation Ax = b with sparse matrix A in COO format
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
void JPCG_coo(void* handle,
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
              const JPCG_Mode mode);

#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

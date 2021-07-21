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

typedef enum JPCG_Mode { JPCG_MODE_FULL = 0, JPCG_MODE_KEEP_NZ_LAYOUT = 1, JPCG_MODE_DO_MAGIC = 99 } JPCG_Mode;

XILINX_PCG_LINKAGE_DECL
void* create_JPCG_handle(const int deviceId, const char* xclbinPath);

XILINX_PCG_LINKAGE_DECL
void destroy_JPCG_handle(void* handle);

XILINX_PCG_LINKAGE_DECL
void JPCG_coo(void* handle,
              const JPCG_Mode mode,
              const uint32_t p_n,
              const uint32_t p_nnz,
              const uint32_t* p_rowIdx,
              const uint32_t* p_colIdx,
              const double* p_data,
              const double* matJ,
              const double* b,
              double* x,
              const uint32_t p_maxIter,
              const double p_tol,
              uint32_t* p_iter,
              double* p_res);

#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

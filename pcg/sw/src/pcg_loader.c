/*
 * Copyright 2020-2021N Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf

#include "pcg.h"
#include <dlfcn.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XILINX_PCG_INLINE_IMPL
#define XILINX_PCG_LINKAGE_DEF inline
#else
#define XILINX_PCG_LINKAGE_DEF
#endif
    
#define XILINX_PCG_SOFILENAME "libXilinxPcgStatic.so"

XILINX_PCG_LINKAGE_DEF
void *xilinx_apps_getCDynamicFunction(const char *funcName) {
    // open the library
    
    void* handle = dlopen(XILINX_PCG_SOFILENAME, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        printf("INFO: %s not loaded. Loading now...\n", XILINX_PCG_SOFILENAME);
        handle = dlopen(XILINX_PCG_SOFILENAME, RTLD_LAZY | RTLD_GLOBAL);
        if (!handle) {
            printf("ERROR: Cannot open library %s: %s."
                "  Please ensure that the library's path is in LD_LIBRARY_PATH.\n", XILINX_PCG_SOFILENAME, dlerror());
            return 0;
        }
    }

    // load the symbol
    dlerror();  // reset errors
    void *pFunc = dlsym(handle, funcName);
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
        printf("ERROR: Cannot load symbol '%s': %s.  Possibly an older version of library %s"
                " is in use.  Please install the correct version.\n", funcName, dlsym_error2, XILINX_PCG_SOFILENAME);
        return 0;
    }
    return pFunc;
}

//#####################################################################################################################

XILINX_PCG_LINKAGE_DEF
void *create_JPCG_handle(int deviceId, const char *xclbinPath) {
    typedef void *(*CreateFunc)(int, const char *);
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps_getCDynamicFunction("create_JPCG_handle");
    if (!pCreateFunc)
        return 0;
    return pCreateFunc(deviceId, xclbinPath);
}

XILINX_PCG_LINKAGE_DEF
void destroy_JPCG_handle(void *handle) {
    typedef void (*DestroyFunc)(void *);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps_getCDynamicFunction("destroy_JPCG_handle");
    if (pDestroyFunc)
        pDestroyFunc(handle);
}

XILINX_PCG_LINKAGE_DEF
void JPCG_coo(void *handle, 
        JPCG_Mode mode, 
        uint32_t p_n,
        uint32_t p_nnz,
        uint32_t *p_rowIdx,
        uint32_t *p_colIdx,
        double *p_data,
        double *matJ,
        double *b,
        double *x,
        uint32_t p_maxIter,
        double p_tol,
        uint32_t *p_iter,
        double *p_res) {
    typedef void (*ApiFunc)(void *, JPCG_Mode, uint32_t, uint32_t, uint32_t*, uint32_t*, double*, double*, double*, double*, uint32_t, double, uint32_t*, double*);
    ApiFunc pApiFunc = (ApiFunc) xilinx_apps_getCDynamicFunction("JPCG_coo");
    if (!pApiFunc)
        return;
    pApiFunc(handle, mode, p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ, b, x, p_maxIter, p_tol, p_iter, p_res);
}

#ifdef __cplusplus
}
#endif


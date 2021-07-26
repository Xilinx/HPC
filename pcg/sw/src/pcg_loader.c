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
XJPCG_Status_t create_JPCG_handle(void **handle, int deviceId, const char *xclbinPath) {
    typedef XJPCG_Status_t (*CreateFunc)(void**, int, const char *);
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps_getCDynamicFunction("create_JPCG_handle");
    if (!pCreateFunc)
        return XJPCG_STATUS_NOT_SUPPORTED;
    return pCreateFunc(handle, deviceId, xclbinPath);
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t destroy_JPCG_handle(void *handle) {
    typedef XJPCG_Status_t (*DestroyFunc)(void *);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps_getCDynamicFunction("destroy_JPCG_handle");
    if (!pDestroyFunc)
        return XJPCG_STATUS_NOT_SUPPORTED;
    return pDestroyFunc(handle);
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_getMetrics(void* handle, XJPCG_Metric_t *metric){
    typedef XJPCG_Status_t (*GetMetrics)(void *, XJPCG_Metric_t *);
    GetMetrics pGetMetrics = (GetMetrics) xilinx_apps_getCDynamicFunction("xJPCG_getMetrics");
    if (pGetMetrics)
        return pGetMetrics(handle, metric);
    else
        return XJPCG_STATUS_NOT_SUPPORTED;
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_peekAtLastStatus(void* handle) {
    typedef XJPCG_Status_t (*PeekAtLastStatus)(void *);
    PeekAtLastStatus pPeekAtLastStatus = (PeekAtLastStatus) xilinx_apps_getCDynamicFunction("xJPCG_peekAtLastStatus");
    if (pPeekAtLastStatus)
        return pPeekAtLastStatus(handle);
    else
        return XJPCG_STATUS_NOT_SUPPORTED;
}

XILINX_PCG_LINKAGE_DEF
const char* xJPCG_getLastMessage(void* handle) {
    typedef const char* (*GetLastMessage)(void *);
    GetLastMessage pGetLastMessage = (GetLastMessage) xilinx_apps_getCDynamicFunction("xJPCG_getLastMessage");
    if (pGetLastMessage)
        return pGetLastMessage(handle);
    else
        return "Function not supported.";
}
XILINX_PCG_LINKAGE_DEF
const char* XJPCG_getErrorString(XJPCG_Status_t code) {
    typedef const char* (*GetErrorString)(XJPCG_Status_t);
    GetErrorString pGetErrorString = (GetErrorString) xilinx_apps_getCDynamicFunction("xJPCG_getErrorString");
    if (pGetErrorString)
        return pGetErrorString(code);
    else
        return "Function not supported.";
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_coo(void *handle, 
        const uint32_t p_n,
        const uint32_t p_nnz,
        const uint32_t *p_rowIdx,
        const uint32_t *p_colIdx,
        const double *p_data,
        const double *matJ,
        const double *b,
        const double *x,
        const uint32_t p_maxIter,
        const double p_tol,
        uint32_t *p_iter,
        double *p_res,
        const XJPCG_Mode mode) {
    typedef XJPCG_Status_t (*ApiFunc)(void *, uint32_t, uint32_t, const uint32_t*, const uint32_t*, const double*, const double*, const double*, const double*, const uint32_t, const double, uint32_t*, double*, const XJPCG_Mode);
    ApiFunc pApiFunc = (ApiFunc) xilinx_apps_getCDynamicFunction("xJPCG_coo");
    if (!pApiFunc)
        return XJPCG_STATUS_NOT_SUPPORTED;
    return pApiFunc(handle, p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ, b, x, p_maxIter, p_tol, p_iter, p_res, mode);
}

#ifdef __cplusplus
}
#endif


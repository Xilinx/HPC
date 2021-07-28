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

#include "pcg.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XILINX_PCG_INLINE_IMPL
#define XILINX_PCG_LINKAGE_DEF inline
#else
#define XILINX_PCG_LINKAGE_DEF
#endif
    
#define XILINX_PCG_SOFILENAME "libXilinxPcgStatic.so"

#define XILINX_APPS_LOADER_ERROR_LENGTH 4096

XILINX_PCG_LINKAGE_DEF
const char *xilinx_apps_manageLoaderError(const char *errorStr) {
    static int s_hasError = 0;  // 1 = we're storing an error, 0 = no error
    static char s_errorStr[XILINX_APPS_LOADER_ERROR_LENGTH];  // storage for error message
    
    // If caller passed in an error message, store it
    if (errorStr) {
        if (strlen(errorStr) > 0) {
            s_hasError = 1;
            (void) strncpy(s_errorStr, errorStr, XILINX_APPS_LOADER_ERROR_LENGTH);
            s_errorStr[XILINX_APPS_LOADER_ERROR_LENGTH - 1] = 0;
        }
        else
            s_hasError = 0;
    }
    
    // No error message argument: return the stored error message if there is one, or null if not
    else
        return (s_hasError) ? s_errorStr : 0;
}


XILINX_PCG_LINKAGE_DEF
void *xilinx_apps_getCDynamicFunction(const char *funcName) {
    char messageBuf[XILINX_APPS_LOADER_ERROR_LENGTH];
    
    // open the library
    
    void* handle = dlopen(XILINX_PCG_SOFILENAME, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
#ifndef NDEBUG
        printf("INFO: %s not loaded. Loading now...\n", XILINX_PCG_SOFILENAME);
#endif
        handle = dlopen(XILINX_PCG_SOFILENAME, RTLD_LAZY | RTLD_GLOBAL);
        if (!handle) {
            snprintf(messageBuf, XILINX_APPS_LOADER_ERROR_LENGTH, "ERROR: Cannot open library %s: %s."
                "  Please ensure that the library's path is in LD_LIBRARY_PATH.\n", XILINX_PCG_SOFILENAME, dlerror());
            (void) xilinx_apps_manageLoaderError(messageBuf);
            return 0;
        }
    }

    // load the symbol
    dlerror();  // reset errors
    void *pFunc = dlsym(handle, funcName);
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
        snprintf(messageBuf, XILINX_APPS_LOADER_ERROR_LENGTH,
            "ERROR: Cannot load symbol '%s': %s.  Possibly an older version of library %s"
            " is in use.  Please install the correct version.\n", funcName, dlsym_error2, XILINX_PCG_SOFILENAME);
        (void) xilinx_apps_manageLoaderError(messageBuf);
        return 0;
    }
    (void) xilinx_apps_manageLoaderError("");
    return pFunc;
}

//#####################################################################################################################

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_createHandle(XJPCG_Handle_t *handle, int deviceId, const char *xclbinPath) {
    typedef XJPCG_Status_t (*CreateFunc)(XJPCG_Handle_t*, int, const char *);
    CreateFunc pCreateFunc = (CreateFunc) xilinx_apps_getCDynamicFunction("xJPCG_createHandle");
    if (!pCreateFunc)
        return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
    return pCreateFunc(handle, deviceId, xclbinPath);
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_destroyHandle(const XJPCG_Handle_t handle) {
    typedef XJPCG_Status_t (*DestroyFunc)(const XJPCG_Handle_t);
    DestroyFunc pDestroyFunc = (DestroyFunc) xilinx_apps_getCDynamicFunction("xJPCG_destroyHandle");
    if (!pDestroyFunc)
        return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
    return pDestroyFunc(handle);
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_getMetrics(const XJPCG_Handle_t handle, XJPCG_Metric_t *metric){
    typedef XJPCG_Status_t (*GetMetrics)(const XJPCG_Handle_t, XJPCG_Metric_t *);
    GetMetrics pGetMetrics = (GetMetrics) xilinx_apps_getCDynamicFunction("xJPCG_getMetrics");
    if (pGetMetrics)
        return pGetMetrics(handle, metric);
    return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_peekAtLastStatus(const XJPCG_Handle_t handle) {
    typedef XJPCG_Status_t (*PeekAtLastStatus)(const XJPCG_Handle_t);
    
    // Check if there was a recent dynamic loading error.  If so, don't try to access the .so during this
    // error handling, but instead return the dynamic loading error
    const char *errorStr = xilinx_apps_manageLoaderError(0);
    if (errorStr)
        return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
    
    PeekAtLastStatus pPeekAtLastStatus = (PeekAtLastStatus) xilinx_apps_getCDynamicFunction("xJPCG_peekAtLastStatus");
    if (pPeekAtLastStatus)
        return pPeekAtLastStatus(handle);
    return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
}

XILINX_PCG_LINKAGE_DEF
const char* xJPCG_getLastMessage(const XJPCG_Handle_t handle) {
    typedef const char* (*GetLastMessage)(const XJPCG_Handle_t);

    // Check if there was a recent dynamic loading error.  If so, don't try to access the .so during this
    // error handling, but instead return the dynamic loading error
    const char *errorStr = xilinx_apps_manageLoaderError(0);
    if (errorStr)
        return errorStr;
    
    GetLastMessage pGetLastMessage = (GetLastMessage) xilinx_apps_getCDynamicFunction("xJPCG_getLastMessage");
    if (pGetLastMessage)
        return pGetLastMessage(handle);

    // If we were unable to get the getLastMessage function, we just generated a new dynamic loading error
    errorStr = xilinx_apps_manageLoaderError(0);
    if (errorStr)
        return errorStr;
    
    // We shouldn't get here
    return "ERROR: Unknown dynamic loading error while calling xJPCG_getLastMessage.";
}

XILINX_PCG_LINKAGE_DEF
const char* xJPCG_getErrorString(XJPCG_Status_t code) {
    typedef const char* (*GetErrorString)(XJPCG_Status_t);

    // Don't try to access the .so when dealing with dynamic loading errors but instead handle them here
    if (code == XJPCG_STATUS_DYNAMIC_LOADING_ERROR)
        return "Dynamic loading error";

    GetErrorString pGetErrorString = (GetErrorString) xilinx_apps_getCDynamicFunction("xJPCG_getErrorString");
    if (pGetErrorString)
        return pGetErrorString(code);
    return "ERROR: Unable to get error string due to dynamic loading error.";
}

XILINX_PCG_LINKAGE_DEF
XJPCG_Status_t xJPCG_cooSolver(const XJPCG_Handle_t handle, 
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
    typedef XJPCG_Status_t (*ApiFunc)(const XJPCG_Handle_t,  uint32_t, uint32_t, const uint32_t*, const uint32_t*, const double*, const double*, const double*, const double*, const uint32_t, const double, uint32_t*, double*, const XJPCG_Mode);
    ApiFunc pApiFunc = (ApiFunc) xilinx_apps_getCDynamicFunction("xJPCG_cooSolver");
    if (!pApiFunc)
        return XJPCG_STATUS_DYNAMIC_LOADING_ERROR;
    return pApiFunc(handle, p_n, p_nnz, p_rowIdx, p_colIdx, p_data, matJ, b, x, p_maxIter, p_tol, p_iter, p_res, mode);
}

#ifdef __cplusplus
}
#endif


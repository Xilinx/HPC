/*
 * Copyright 2020 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Thanks to Aaron Isotton for his dynamic loading ideas in https://tldp.org/HOWTO/pdf/C++-dlopen.pdf

#include "pcg.h"
#include <string>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <exception>

namespace xilinx_apps {
namespace pcg {

// TODO: move to pcg.hpp

/**
 * @brief %Exception class for PCG run-time errors
 * 
 * This exception class is derived from `std::exception` and provides the standard @ref what() member function.
 * An object of this class is constructed with an error message string, which is stored internally and
 * retrieved with the @ref what() member function.
 */
class Exception : public std::exception {
    std::string message;
public:
    /**
     * Constructs an Exception object.
     * 
     * @param msg an error message string, which is copied and stored internal to the object
     */
    Exception(const std::string &msg) : message(msg) {}
    
    /**
     * Returns the error message string passed to the constructor.
     * 
     * @return the error message string
     */
    virtual const char* what() const noexcept override { return message.c_str(); }
};

}
}


namespace {

void *getDynamicFunction(const std::string &funcName) {
    // open the library
    
    std::string SOFILEPATH = "libXilinxPcgStatic.so";
    void* handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    if (handle == nullptr) {
        std::cout << "INFO: " << SOFILEPATH << " not loaded. Loading now..." << std::endl;
        handle = dlopen(SOFILEPATH.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        std::cout << "DEBUG: after dlopen" << std::endl;
        if (handle == nullptr) {
            std::cout << "DEBUG: inside handle==nullptr" << std::endl;
            std::ostringstream oss;
            oss << "Cannot open library " << SOFILEPATH << ": " << dlerror()
                    << ".  Please ensure that the library's path is in LD_LIBRARY_PATH."  << std::endl;
            std::cout << "DEBUG: after oss filling" << std::endl;
            throw xilinx_apps::pcg::Exception(oss.str());
        }
    }

    // load the symbol
    std::cout << "DEBUG: after handle==nullptr check" << std::endl;
    dlerror();  // reset errors
    std::cout << "DEBUG: before dlsym" << std::endl;
    void *pFunc = dlsym(handle, funcName.c_str());
    std::cout << "DEBUG: after dlsym" << std::endl;
    const char* dlsym_error2 = dlerror();
    if (dlsym_error2) {
        std::cout << "DEBUG: inside dlsym_error2" << std::endl;
        std::ostringstream oss;
        oss << "Cannot load symbol '" << funcName << "': " << dlsym_error2
                << ".  Possibly an older version of library " << SOFILEPATH
                << " is in use.  Please install the correct version." << std::endl;
        std::cout << "DEBUG: after 2nd oss filling" << std::endl;
        throw xilinx_apps::pcg::Exception(oss.str());
    }
    std::cout << "DEBUG: before return" << std::endl;
    return pFunc;
}

}  // namespace <anonymous>

//#####################################################################################################################

extern "C" {

#ifdef XILINX_PCG_INLINE_IMPL
#define XILINX_PCG_LINKAGE_DEF inline
#else
#define XILINX_PCG_LINKAGE_DEF
#endif
    
XILINX_PCG_LINKAGE_DEF
void *create_JPCG_handle(int deviceId, const char *xclbinPath) {
    typedef void *(*CreateFunc)(int, const char *);
    CreateFunc pCreateFunc = (CreateFunc) getDynamicFunction("create_JPCG_handle");
    return pCreateFunc(deviceId, xclbinPath);
}

XILINX_PCG_LINKAGE_DEF
void destroy_JPCG_handle(void *handle) {
    typedef void (*DestroyFunc)(void *);
    DestroyFunc pDestroyFunc = (DestroyFunc) getDynamicFunction("destroy_JPCG_handle");
    return pDestroyFunc(handle);
}

XILINX_PCG_LINKAGE_DEF
void JPCG_coo(void *handle, JPCG_Mode mode, int dummy) {
    typedef void (*ApiFunc)(void *, JPCG_Mode, int);
    ApiFunc pApiFunc = (ApiFunc) getDynamicFunction("JPCG_coo");
    return pApiFunc(handle, mode, dummy);
}


}  // extern "C"


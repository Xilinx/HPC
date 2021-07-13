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

#ifndef PCG_HPP
#define PCG_HPP

#include "impl/pcgImp.hpp"

namespace xilinx_apps {
namespace pcg {

/**
 * @brief Simple string class for avoiding ABI problems
 *
 * This class provides string memory management like `std::string` but without ABI compatibility issues.
 * ABI (Application Binary Interface) problems can arise when using standard C++ classes in a coding environment
 * that uses multiple compiler versions.  For example, if you compile your application code using a different
 * version of the g++ compiler from the one used to compile this library, then when using dynamic loading,
 * standard C++ types, such as `std::string`, may not pass correctly from your code into the library.
 * This string class avoids these compatibility issues by using only plain data types internally.
 */
class XString {
   public:
    XString() = default;
    ~XString() { clear(); }
    XString(const XString& other) { copyIn(other.data); }
    XString(XString&& other) { steal(std::forward<XString>(other)); }
    XString(const char* cstr) { copyIn(cstr); }
    XString(const std::string& str) { copyIn(str.c_str()); }
    XString& operator=(const XString& other) {
        copyIn(other.data);
        return *this;
    }
    XString& operator=(XString&& other) {
        steal(std::forward<XString>(other));
        return *this;
    }
    XString& operator=(const char* cstr) {
        copyIn(cstr);
        return *this;
    }
    XString& operator=(const std::string& str) {
        copyIn(str.c_str());
        return *this;
    }
    operator std::string() const { return std::string(c_str()); }
    operator const char*() const { return c_str(); }
    const char* c_str() const { return data == nullptr ? "" : data; }
    bool empty() const { return data == nullptr || std::strlen(data) == 0; }

    bool operator==(const XString& other) const {
        if (data == nullptr && other.data == nullptr) return true;
        if (data == nullptr || other.data == nullptr) return false;
        return std::strcmp(data, other.data) == 0;
    }

    void clear() {
        if (data != nullptr) std::free(data);
        data = nullptr;
    }

   private:
    char* data = nullptr;

    void copyIn(const char* other) {
        clear();
        if (other != nullptr) {
            data = static_cast<char*>(std::malloc(std::strlen(other) + 1));
            std::strcpy(data, other);
        }
    }

    void steal(XString&& other) {
        clear();
        data = other.data;
        other.data = nullptr;
    }
};

struct Options {
    int deviceId;
    XString xclbinName;
};

template <typename t_DataType>
class PCG {
   public:
    PCG(){};

    PCG(Options& l_options) { pImpl.init(l_options.deviceId, l_options.xclbinName); }

    void setCooMat(uint32_t p_dim, uint32_t p_nnz, uint32_t* p_rowIdx, uint32_t* p_colIdx, t_DataType* p_data) {
        pImpl.setCooMat(p_dim, p_nnz, p_rowIdx, p_colIdx, p_data);
    }

    void setCscSymMat(uint32_t p_dim, uint32_t p_nnz, uint32_t* p_rowIdx, uint32_t* p_colPtr, t_DataType* p_data) {
        pImpl.setCscSymMat(p_dim, p_nnz, p_rowIdx, p_colPtr, p_data);
    }
    int updateMat(uint32_t p_dim, uint32_t p_nnz, t_DataType* p_data) { return pImpl.updateMat(p_dim, p_nnz, p_data); }
    void setVec(uint32_t p_dim, t_DataType* p_b, t_DataType* p_diagA) { pImpl.setVec(p_dim, p_b, p_diagA); }

    Results<t_DataType> run(unsigned int p_maxIter, t_DataType p_tol) { return pImpl.run(p_maxIter, p_tol); }

    std::vector<uint32_t> getMatInfo() { return pImpl.getMatInfo(); }

    t_DataType getDot() { return pImpl.getDot(); }

    CgVector getRes() { return pImpl.getRes(); }
    void sendVec() { pImpl.sendVec(); }

    void setInstr(unsigned int p_maxIter, t_DataType p_tol) { pImpl.setInstr(p_maxIter, p_tol); }

   private:
    PCGImpl<t_DataType, 4, 64, 8, 16, 4096, 4096, 256> pImpl;
};

}
}

#endif

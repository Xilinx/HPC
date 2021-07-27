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

#ifndef SPM_EXCEPTION_HPP
#define SPM_EXCEPTION_HPP

namespace xf {
namespace sparse {

typedef enum {
    XSPM_STATUS_SUCCESS,           // success status
    XSPM_STATUS_ALLOC_FAILED,      // host memory allocation failure
    XSPM_STATUS_INVALID_VALUE,     // invalid parameters
    XSPM_STATUS_PARTITION_FAILED,  // matrix partition failed
    XSPM_STATUS_INTERNAL_ERROR,    // internal errors or bugs
    XSPM_STATUS_NOT_SUPPORTED      // unsupported behavior
} XSPM_Status_t;

class SpmException : public std::exception {
   public:
    SpmException(const std::string str, const XSPM_Status_t p_stat) : m_msg(str), m_status(p_stat) {}

    const char* what() const noexcept override { 
        return m_msg.c_str(); 
    }

    XSPM_Status_t getStatus() const { 
        return m_status; 
    }

   protected:
    std::string m_msg;
    XSPM_Status_t m_status;
};

class SpmInternalError : public SpmException {
   public:
    SpmInternalError(std::string str) : SpmException("INTERNAL ERROR: " + str +"\n", XSPM_STATUS_INTERNAL_ERROR) {}
};

class SpmAllocFailed : public SpmException {
   public:
    SpmAllocFailed(std::string str) : SpmException("Alloc Failed ERROR: " + str+"\n", XSPM_STATUS_ALLOC_FAILED) {}
};

class SpmInvalidValue : public SpmException {
   public:
    SpmInvalidValue(std::string str) : SpmException("INVALID VALUE ERROR: " + str+"\n", XSPM_STATUS_INVALID_VALUE) {}
};

class SpmParFailed : public SpmException {
   public:
    SpmParFailed(std::string str) : SpmException("Matrix Partition ERROR: " + str+"\n", XSPM_STATUS_PARTITION_FAILED) {}
};

class SpmNotSupported : public SpmException {
   public:
    SpmNotSupported(std::string str) : SpmException("Matrix Operation ERROR: " + str+"\n", XSPM_STATUS_NOT_SUPPORTED) {}
};
}
}
#endif

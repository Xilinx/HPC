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

#ifndef PCG_EXCEPTION_HPP
#define PCG_EXCEPTION_HPP

namespace xilinx_apps {
namespace pcg {

class CgException : public std::exception {
   public:
    CgException(const std::string str, const XJPCG_Status_t p_stat) : m_msg(str), m_status(p_stat) {}

    const char* what() const noexcept override { return m_msg.c_str(); }

    XJPCG_Status_t getStatus() const { return m_status; }

   protected:
    std::string m_msg;
    XJPCG_Status_t m_status;
};

class CgExecutionFailed : public CgException {
   public:
    CgExecutionFailed(std::string str) : CgException("Solver ERROR: " + str, XJPCG_STATUS_EXECUTION_FAILED) {}
};

class CgInternalError : public CgException {
   public:
    CgInternalError(std::string str) : CgException("INTERNAL ERROR: " + str, XJPCG_STATUS_INTERNAL_ERROR) {}
};

class CgAllocFailed : public CgException {
   public:
    CgAllocFailed(std::string str) : CgException("Alloc Failed ERROR: " + str, XJPCG_STATUS_ALLOC_FAILED) {}
};

class CgInvalidValue : public CgException {
   public:
    CgInvalidValue(std::string str) : CgException("INVALID VALUE ERROR: " + str, XJPCG_STATUS_INVALID_VALUE) {}
};
}
}
#endif

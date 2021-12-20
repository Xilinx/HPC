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

#ifndef XANS_EXCEPTION_HPP
#define XANS_EXCEPTION_HPP

namespace xilinx_apps {
namespace xans {

typedef enum {
    XANS_STATUS_SUCCESS,        // success status
    XANS_STATUS_FAILED_CU_INIT, // failed initializing CUs
    XANS_STATUS_INVALID_IP,     // invalid IP address
    XANS_STATUS_INVALID_VALUE   // invalid parameter value
} XANS_Status_t;

class xansException : public std::exception {
   public:
    xansException(const std::string str, const XANS_Status_t p_stat) : m_msg(str), m_status(p_stat) {}

    const char* what() const noexcept override { return m_msg.c_str(); }

    XANS_Status_t getStatus() const { return m_status; }

   protected:
    std::string m_msg;
    XANS_Status_t m_status;
};

class xansFailedCUInit : public xansException {
   public:
    xansFailedCUInit(std::string str) : xansException("CU INIT ERROR: " + str + "\n", XANS_STATUS_FAILED_CU_INIT) {}
};

class xansInvalidValue : public xansException {
   public:
    xansInvalidValue(std::string str) : xansException("ERROR: " + str + "\n", XANS_STATUS_INVALID_VALUE) {}
};

class xansInvalidIp : public xansException {
   public:
    xansInvalidIp(std::string str) : xansException("IP Address ERROR: " + str + "\n", XANS_STATUS_INVALID_IP) {}
};
}
}
#endif

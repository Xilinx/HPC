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

#ifndef CMAC_HPP
#define CMAC_HPP

#include "sw/xNativeFPGA.hpp"
#include <map>
#include <string>

namespace xilinx_apps {
namespace xans {

constexpr size_t stat_tx_status = 0x0200;
constexpr size_t stat_rx_status = 0x0204;

class KernelCMAC : public xilinx_apps::hpc_common::IP {
   public:
    KernelCMAC() = default;
    void initCU(const unsigned int p_id);
    std::map<std::string, bool> linkStatus();
};
}
}

#endif

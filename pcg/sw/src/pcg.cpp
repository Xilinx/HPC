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
#include "impl/pcgImp.hpp"

using PcgImpl = xilinx_apps::pcg::PCGImpl<double, 4, 64, 8, 16, 4096, 4096, 256>;

extern "C" {

void *create_JPCG_handle(int deviceId, const char *xclbinPath) {
    PcgImpl *pImpl = new PcgImpl();
    pImpl->init(deviceId, xclbinPath);
    return pImpl;
}

void destroy_JPCG_handle(void *handle) {
    auto pImpl = reinterpret_cast<PcgImpl *>(handle);
    delete pImpl;
}

void JPCG_coo(void *handle, JPCG_Mode mode, int dummy) {
    (void) handle;
    (void) mode;
    (void) dummy;
}

}
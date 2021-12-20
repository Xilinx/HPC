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

#include "krnl_transY.hpp"

extern "C" void krnl_transY(const uint32_t p_startAddr,
                           const uint32_t p_rows,
                           hls::stream<ap_uint<SPARSE_dataBits> >& p_inDatStr,
                           hls::stream<ap_uint<XANS_netDataBits> >& p_outNetDatStr) {

    SCALAR(p_startAddr)
    SCALAR(p_rows)
    AXIS(p_inDatStr)
    AXIS(p_outNetDatStr)
    SCALAR(return)

    xilinx_apps::dspmv::transY<SPARSE_dataBits, XANS_netDataBits>(p_startAddr,
                                                                  p_rows,
                                                                  p_inDatStr,
                                                                  p_outNetDatStr);
}

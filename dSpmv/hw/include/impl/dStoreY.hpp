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

/**
 *  @file dStoreY.hpp
 *  @brief distributed storeY Level 1 template function implementation.
 *
 */

#ifndef XILINX_APPS_DSPMV_DSTOREY_HPP
#define XILINX_APPS_DSPMV_DSTOREY_HPP

#include "ap_int.h"
#include "hls_stream.h"
#include "xf_blas.hpp"

namespace xilinx_apps {
namespace dspmv {

template <unsigned int t_DataBits, 
          unsigned int t_NetDataBits,
          unsigned int t_MemBits>
void dStoreY(const uint32_t p_numNodes,
            hls::stream<ap_uint<t_NetDataBits> >& p_inNetDatStr,
            ap_uint<t_MemBits>* p_resPtr) {
    constexpr unsigned int t_NetInts = t_NetDataBits / (8*sizeof(uint32_t));
    constexpr unsigned int t_NetDataNum = t_NetDataBits / t_DataBits;
    constexpr unsigned int t_NetMemDataNum = t_NetDataBits / t_MemBits;

    for (unsigned int i=0; i<p_numNodes; ++i) {
        xf::blas::WideType<uint32_t, t_NetInts> l_param = p_inNetDatStr.read();;
        uint32_t l_startAddr = l_param[0]; //start address of ap_uint<t_DataBits>
        uint32_t l_rows = l_param[1];
        unsigned int l_rowBks = (l_rows + t_NetDataNum - 1) / t_NetDataNum;
        ap_uint<t_MemBits>* l_memPtr = p_resPtr;
        l_memPtr += l_startAddr;

LOOP_REC_NETDATA:
        for (unsigned int j=0; j<l_rowBks; ++j) {
#pragma HLS PIPELINE II=t_NetMemDataNum
            ap_uint<t_NetDataBits> l_dat = p_inNetDatStr.read();
LOOP_WRITE_MEM:
            for (unsigned int k=0; k<t_NetMemDataNum; ++k) {
#pragma HLS PIPELINE II=1
                ap_uint<t_MemBits> l_val = l_dat.range(t_MemBits-1, 0);
                l_dat = l_dat >> t_MemBits;
                l_memPtr[0] = l_val;
                l_memPtr++;
            }
        }
    }
}

} //end namespace xilinx_apps
} //end namespace dsonv
#endif

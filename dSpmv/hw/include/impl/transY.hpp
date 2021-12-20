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
 *  @file transY.hpp
 *  @brief distributed SPMV Level 1 template function implementation.
 *
 */

#ifndef XILINX_APPS_DSPMV_TRANSY_HPP
#define XILINX_APPS_DSPMV_TRANSY_HPP

#include "ap_int.h"
#include "hls_stream.h"
#include "xf_blas.hpp"

namespace xilinx_apps {
namespace dspmv {

template <unsigned int t_DataBits, 
          unsigned int t_NetDataBits>
void transY(const uint32_t p_startAddr, //start addr of memory, unit is memory word width
            const uint32_t p_rows, //must be mutiple of number of data stored in one memory unit
            hls::stream<ap_uint<t_DataBits> >& p_inDatStr,
            hls::stream<ap_uint<t_NetDataBits> >& p_outNetDatStr) {
    constexpr unsigned int t_NetInts = t_NetDataBits / (8*sizeof(uint32_t));
    constexpr unsigned int t_NetDataNum = t_NetDataBits / t_DataBits;

    xf::blas::WideType<uint32_t, t_NetInts> l_param;
    l_param[0] = p_startAddr;
    l_param[1] = p_rows;
    p_outNetDatStr.write(l_param);
    uint32_t l_rowBks = p_rows / t_NetDataNum;
    uint32_t l_alignedRows = (p_rows % t_NetDataNum == 0)? l_rowBks * t_NetDataNum: (l_rowBks+1)*t_NetDataNum;
    ap_uint<t_NetDataBits> l_val;

LOOP_TRAN_DATA:
    for (unsigned int i=0; i < l_alignedRows; ++i) {
#pragma HLS PIPELINE II=1
        uint8_t l_iMod = i % t_NetDataNum;
        ap_uint<t_DataBits> l_inDat = 0;
        if ((i != 0) && ( l_iMod == 0)) {
            p_outNetDatStr.write(l_val);
        }
        if (i < p_rows) {
            l_inDat = p_inDatStr.read(); 
        }
        l_val = (l_val >> t_DataBits);
        l_val.range(t_NetDataBits-1, t_NetDataBits-t_DataBits) = l_inDat;
    }
    p_outNetDatStr.write(l_val);
}

} // end namespace dsmpv
} // end namespace xilinx_apps

#endif

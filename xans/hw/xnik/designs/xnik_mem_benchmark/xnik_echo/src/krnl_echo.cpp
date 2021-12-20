
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
#include <cstdint>
#include "hw/interface.hpp"
#include "ap_int.h"
#include "hls_stream.h"

extern "C" void krnl_echo(hls::stream<ap_uint<XANS_netDataBits> >& p_inStr,
                          uint32_t* p_memPtr,
                          unsigned int p_numWords,
                          hls::stream<ap_uint<XANS_netDataBits> >& p_outStr) { //number of words. Each word has XANS_netDataBits
    AXIS(p_inStr)
    AXIS(p_outStr)
    POINTER(p_memPtr, p_memPtr)
    SCALAR(p_numWords)
    SCALAR(return)

    uint32_t l_errs = 0;
    for (unsigned int i=0; i<p_numWords; ++i) {
#pragma HLS PIPELINE II=1
        ap_uint<XANS_netDataBits> l_val = p_inStr.read(); 
        if (l_val != i) {
            l_errs++;
        } 
    }
    if (l_errs == 0) {
        for (unsigned int i=0; i<p_numWords; ++i) {
    #pragma HLS PIPELINE II=1
            ap_uint<XANS_netDataBits> l_val = i;
            p_outStr.write(l_val);
        }
    }
    p_memPtr[0] = l_errs;
}

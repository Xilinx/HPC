
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
#include "hw/interface.hpp"
#include "ap_int.h"
#include "hls_stream.h"

extern "C" void krnl_mm2s(hls::stream<ap_uint<XANS_netDataBits> >& p_outStr,
                          hls::stream<ap_uint<1> >& p_outCtl,
                          unsigned int p_numWords) { //number of words. Each word has XANS_netDataBits
    AXIS(p_outStr)
    AXIS(p_outCtl)
    SCALAR(p_numWords)
    SCALAR(return)

LOOP_SEND:
    for (unsigned int i=0; i<p_numWords; ++i) {
#pragma HLS PIPELINE II=1
        ap_uint<XANS_netDataBits> l_val = i;
        p_outStr.write(l_val);
        if ((i == 0) || (i == (p_numWords-1))) {
            p_outCtl.write(1);
        }
    }
}

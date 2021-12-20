
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

extern "C" void krnl_ctrl(ap_uint<XANS_netDataBits>* p_memPtr,
                          hls::stream<ap_uint<1> >& p_outStr,
                          hls::stream<ap_uint<XANS_netDataBits> >& p_inStr) { //number of words. Each word has XANS_netDataBits
    POINTER(p_memPtr, p_memPtr)
    AXIS(p_outStr)
    AXIS(p_inStr)
    SCALAR(return)

    ap_uint<XANS_netDataBits> l_val = p_memPtr[0];
    if (l_val[0] == 1) {
        p_outStr.write(1);
        p_memPtr[1] = p_inStr.read();
    }
}

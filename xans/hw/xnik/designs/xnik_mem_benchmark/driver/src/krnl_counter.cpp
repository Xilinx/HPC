
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

extern "C" void krnl_counter(hls::stream<ap_uint<1> >& p_inCtlM2ss,
                          hls::stream<ap_uint<1> >& p_inCtlS2mm,
                          uint64_t* p_memPtr) { 
    AXIS(p_inCtlM2ss)
    AXIS(p_inCtlS2mm)
    POINTER(p_memPtr, p_memPtr)
    SCALAR(return)

    uint64_t l_latCycles = 0;
    uint64_t l_sendCycles = 0;
    uint64_t l_recCycles = 0;
    uint64_t l_totalCycles = 0;

    ap_uint<1> l_start = p_inCtlM2ss.read();
    bool l_exit = false;
    bool l_endSend = false;
    bool l_startRec = false;
    bool l_endRec = false;

LOOP_COUNTER:
    while (!l_exit) {
#pragma HLS PIPELINE II=1
        ap_uint<1> l_unused;
        l_totalCycles++;
        if (!l_endSend) {
            l_endSend = p_inCtlM2ss.read_nb(l_unused);
            l_sendCycles++;
        }
        if (!l_startRec) {
            l_startRec = p_inCtlS2mm.read_nb(l_unused);
            l_latCycles++;
        }
        else {
            l_endRec = p_inCtlS2mm.read_nb(l_unused);
            l_recCycles++;
        }
        l_exit = l_endSend && l_endRec;
    }
    p_memPtr[0] = l_latCycles;
    p_memPtr[1] = l_sendCycles;
    p_memPtr[2] = l_recCycles;
    p_memPtr[3] = l_totalCycles;
    
}

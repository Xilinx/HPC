/*
 * Copyright 2019 Xilinx, Inc.
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

#include "interface.hpp"
#include "ap_utils.h"
#include "fcnKernel.hpp"
#include "fcnInstr.hpp"
#include "kernelProcess.hpp"
#include "timer.hpp"

extern "C" void krnl_fcn(uint8_t* p_instr,
                         HPC_interface* p_W0,
                         HPC_interface* p_W1,
                         HPC_interface* p_W2,
                         HPC_interface* p_W3,
#if HPC_numChannels > 4
                         HPC_interface* p_W4,
                         HPC_interface* p_W5,
                         HPC_interface* p_W6,
                         HPC_interface* p_W7,
#endif
#if HPC_numChannels > 8
                         HPC_interface* p_W8,
                         HPC_interface* p_W9,
                         HPC_interface* p_Wa,
                         HPC_interface* p_Wb,
                         HPC_interface* p_Wc,
                         HPC_interface* p_Wd,
                         HPC_interface* p_We,
                         HPC_interface* p_Wf,
#endif
                         HPC_interface* p_vecIn0,
                         HPC_interface* p_vecOut0,
#if HPC_vecChannels > 1
                         HPC_interface* p_vecIn1,
                         HPC_interface* p_vecOut1,
#endif
#if HPC_vecChannels > 2
                         HPC_interface* p_vecIn2,
                         HPC_interface* p_vecOut2,
#endif
#if HPC_vecChannels > 3
                         HPC_interface* p_vecIn3,
                         HPC_interface* p_vecOut3,
#endif
                         HPC_biasInterface* p_bias) {

    POINTER(p_instr, gmem_instr)
    POINTER(p_W0, gmem_A0)
    POINTER(p_W1, gmem_A1)
    POINTER(p_W2, gmem_A2)
    POINTER(p_W3, gmem_A3)
#if HPC_numChannels > 4
    POINTER(p_W4, gmem_A4)
    POINTER(p_W5, gmem_A5)
    POINTER(p_W6, gmem_A6)
    POINTER(p_W7, gmem_A7)
#endif
#if HPC_numChannels > 8
    POINTER(p_W8, gmem_A8)
    POINTER(p_W9, gmem_A9)
    POINTER(p_Wa, gmem_Aa)
    POINTER(p_Wb, gmem_Ab)
    POINTER(p_Wc, gmem_Ac)
    POINTER(p_Wd, gmem_Ad)
    POINTER(p_We, gmem_Ae)
    POINTER(p_Wf, gmem_Af)
#endif
    POINTER(p_vecIn0, gmem_vecIn0)
    POINTER(p_vecOut0, gmem_vecOut0)
#if HPC_vecChannels > 1
    POINTER(p_vecIn1, gmem_vecIn1)
    POINTER(p_vecOut1, gmem_vecOut1)
#endif
#if HPC_vecChannels > 2
    POINTER(p_vecIn2, gmem_vecIn2)
    POINTER(p_vecOut2, gmem_vecOut2)
#endif
#if HPC_vecChannels > 3
    POINTER(p_vecIn3, gmem_vecIn3)
    POINTER(p_vecOut3, gmem_vecOut3)
#endif
    POINTER(p_bias, gmem_bias)
    SCALAR(return )

    hls::stream<xf::hpc::Clock_t> p_clock;
    hls::stream<xf::hpc::Signal_t> p_signal;
#pragma HLS DATAFLOW

#ifdef __SYNTHESIS__
    timer(p_signal, p_clock);
#endif

    parseInstr(p_signal, p_clock, p_instr, p_bias, p_vecIn0, p_vecOut0,
#if HPC_vecChannels > 1
               p_vecIn1, p_vecOut1,
#endif
#if HPC_vecChannels > 2
               p_vecIn2, p_vecOut2,
#endif
#if HPC_vecChannels > 3
               p_vecIn3, p_vecOut3,
#endif
               p_W0, p_W1, p_W2, p_W3
#if HPC_numChannels > 4
               ,
               p_W4, p_W5, p_W6, p_W7
#endif
#if HPC_numChannels > 8
               ,
               p_W8, p_W9, p_Wa, p_Wb, p_Wc, p_Wd, p_We, p_Wf
#endif
               );
}

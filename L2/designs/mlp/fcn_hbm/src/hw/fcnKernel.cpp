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

extern "C" void krnl_fcn(HPC_interface* p_instr,
                         HPC_interface* p_A0,
                         HPC_interface* p_A1,
                         HPC_interface* p_A2,
                         HPC_interface* p_A3,
#if HPC_numChannels > 4
                         HPC_interface* p_A4,
                         HPC_interface* p_A5,
                         HPC_interface* p_A6,
                         HPC_interface* p_A7,
#endif
#if HPC_numChannels > 8
                         HPC_interface* p_A8,
                         HPC_interface* p_A9,
                         HPC_interface* p_Aa,
                         HPC_interface* p_Ab,
                         HPC_interface* p_Ac,
                         HPC_interface* p_Ad,
                         HPC_interface* p_Ae,
                         HPC_interface* p_Af,
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
                         HPC_interface* p_bias) {

    POINTER(p_instr, gmem_instr)
    POINTER(p_A0, gmem_A0)
    POINTER(p_A1, gmem_A1)
    POINTER(p_A2, gmem_A2)
    POINTER(p_A3, gmem_A3)
#if HPC_numChannels > 4
    POINTER(p_A4, gmem_A4)
    POINTER(p_A5, gmem_A5)
    POINTER(p_A6, gmem_A6)
    POINTER(p_A7, gmem_A7)
#endif
#if HPC_numChannels > 8
    POINTER(p_A8, gmem_A8)
    POINTER(p_A9, gmem_A9)
    POINTER(p_Aa, gmem_Aa)
    POINTER(p_Ab, gmem_Ab)
    POINTER(p_Ac, gmem_Ac)
    POINTER(p_Ad, gmem_Ad)
    POINTER(p_Ae, gmem_Ae)
    POINTER(p_Af, gmem_Af)
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
               p_A0, p_A1, p_A2, p_A3
#if HPC_numChannels > 4
               ,
               p_A4, p_A5, p_A6, p_A7
#endif
#if HPC_numChannels > 8
               ,
               p_A8, p_A9, p_Aa, p_Ab, p_Ac, p_Ad, p_Ae, p_Af
#endif
               );
}

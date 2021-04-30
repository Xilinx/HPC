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

template <typename... Ts>
void parseInstr(hls::stream<xf::hpc::Signal_t>& p_signal,
                hls::stream<xf::hpc::Clock_t>& p_clock,
                HPC_interface* p_instr,
                Ts... p_As) {
    static_assert(0 == (8 * HPC_instrBytes) % HPC_wideType::t_TypeWidth, "");
    xf::hpc::mlp::FcnInstr<HPC_instrBytes> fcnInstr;
#ifdef __SYNTHESIS__
    p_signal.write(xf::hpc::START);
#endif
    for (int i = 0; i < HPC_maxInstrs; i++) {
        fcnInstr.template load<HPC_interface>(p_instr + i * HPC_instrBytes * 8 / HPC_wideType::t_TypeWidth);
        int l_outVecSize = fcnInstr.getOutVecSize();
        int l_inVecSize = fcnInstr.getInVecSize();
        int p_batch = fcnInstr.getBatch() / HPC_vecChannels;
        int p_matrixOffset = fcnInstr.getWeightsOffset() * 8 / HPC_wideType::t_TypeWidth;
        int p_inputOffset = fcnInstr.getInputOffset() * 8 / HPC_wideType::t_TypeWidth;
        int p_biasOffset = fcnInstr.getBiasOffset() * 8 / HPC_wideType::t_TypeWidth;
        int p_outputOffset = fcnInstr.getOutputOffset() * 8 / HPC_wideType::t_TypeWidth;
        xf::hpc::mlp::ActFunc_t p_act = fcnInstr.getActivation();

        if (p_batch == 0) break;
        kernelProcess<HPC_interface, HPC_parEntries, HPC_numChannels, HPC_vecChannels>(
            p_batch, l_outVecSize, l_inVecSize, p_matrixOffset, p_inputOffset, p_biasOffset, p_outputOffset, p_act,
            p_As...);
        uint64_t l_clock = 0;
#ifdef __SYNTHESIS__
        ap_wait();
        p_signal.write(xf::hpc::STAMP);
        ap_wait();
        l_clock = p_clock.read();
#endif
        fcnInstr.setClock(l_clock);
        fcnInstr.template store<HPC_interface>(p_instr + i * HPC_instrBytes * 8 / HPC_wideType::t_TypeWidth);
    }

#ifdef __SYNTHESIS__
    p_signal.write(xf::hpc::STOP);
#endif
}

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
#endif
#if HPC_numChannels > 10
                         HPC_interface* p_Aa,
                         HPC_interface* p_Ab,
#endif
#if HPC_numChannels > 12
                         HPC_interface* p_Ac,
                         HPC_interface* p_Ad,
                         HPC_interface* p_Ae,
                         HPC_interface* p_Af,
#endif
#if HPC_numChannels > 16
                         HPC_interface* p_A10,
                         HPC_interface* p_A11,
                         HPC_interface* p_A12,
                         HPC_interface* p_A13,
#endif
#if HPC_numChannels > 20
                         HPC_interface* p_A14,
                         HPC_interface* p_A15,
                         HPC_interface* p_A16,
                         HPC_interface* p_A17,
#endif
#if HPC_numChannels > 24
                         HPC_interface* p_A18,
#endif
#if HPC_numChannels > 25
                         HPC_interface* p_A19,
                         HPC_interface* p_A1a,
                         HPC_interface* p_A1b,
#endif
#if HPC_numChannels > 28
                         HPC_interface* p_A1c,
                         HPC_interface* p_A1d,
                         HPC_interface* p_A1e,
                         HPC_interface* p_A1f,
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
#endif
#if HPC_numChannels > 12
    POINTER(p_Ac, gmem_Ac)
    POINTER(p_Ad, gmem_Ad)
    POINTER(p_Ae, gmem_Ae)
    POINTER(p_Af, gmem_Af)
#endif
#if HPC_numChannels > 16
    POINTER(p_A10, gmem_A10)
    POINTER(p_A11, gmem_A11)
    POINTER(p_A12, gmem_A12)
    POINTER(p_A13, gmem_A13)
#endif
#if HPC_numChannels > 20
    POINTER(p_A14, gmem_A14)
    POINTER(p_A15, gmem_A15)
    POINTER(p_A16, gmem_A16)
    POINTER(p_A17, gmem_A17)
#endif
#if HPC_numChannels > 24
    POINTER(p_A18, gmem_A18)
#endif
#if HPC_numChannels > 25
    POINTER(p_A19, gmem_A19)
    POINTER(p_A1a, gmem_A1a)
    POINTER(p_A1b, gmem_A1b)
#endif
#if HPC_numChannels > 28
    POINTER(p_A1c, gmem_A1c)
    POINTER(p_A1d, gmem_A1d)
    POINTER(p_A1e, gmem_A1e)
    POINTER(p_A1f, gmem_A1f)
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
               p_A8, p_A9, p_Aa, p_Ab
#endif
#if HPC_numChannels > 12
               ,
               p_Ac, p_Ad, p_Ae, p_Af
#endif
#if HPC_numChannels > 16
               ,
               p_A10, p_A11, p_A12, p_A13
#endif
#if HPC_numChannels > 20
               ,
               p_A14, p_A15, p_A16, p_A17
#endif
#if HPC_numChannels > 24
               ,
               p_A18
#endif
#if HPC_numChannels > 25
               ,
               p_A19, p_A1a, p_A1b
#endif
#if HPC_numChannels > 28
               ,
               p_A1c, p_A1d, p_A1e, p_A1f
#endif
               );
}

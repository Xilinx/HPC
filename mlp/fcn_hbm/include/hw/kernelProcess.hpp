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
#include "fcnKernel.hpp"
#include "fcn.hpp"
#include "doubleBuffer.hpp"
#include "loadWeights.hpp"
#include "timer.hpp"

#ifndef __HPC_FCN_PROC_HPP__
#define __HPC_FCN_PROC_HPP__

template <typename t_Interface, int t_ParEntries, int t_WeightChannels, int t_VecChannels, typename... Ts>
void kernelProcess(uint32_t p_batch,
                   uint32_t p_outVecSize,
                   uint32_t p_inVecSize,
                   uint32_t p_weightOffset,
                   uint32_t p_inputOffset,
                   uint32_t p_biasOffset,
                   uint32_t p_outputOffset,
                   xf::hpc::mlp::ActFunc_t p_act,
                   HPC_biasInterface* p_bias,
                   t_Interface* p_vecIn0,
                   t_Interface* p_vecOut0,
#if HPC_vecChannels > 1
                   t_Interface* p_vecIn1,
                   t_Interface* p_vecOut1,
#endif
#if HPC_vecChannels > 2
                   t_Interface* p_vecIn2,
                   t_Interface* p_vecOut2,
#endif
#if HPC_vecChannels > 3
                   t_Interface* p_vecIn3,
                   t_Interface* p_vecOut3,
#endif
                   Ts... p_As) {
    hls::stream<t_Interface> l_strA[t_WeightChannels];
#pragma HLS ARRAY_PARTITION variable = l_strA dim = 1 complete
    hls::stream<HPC_biasInterface> l_bias;
    hls::stream<t_Interface> l_vecIn[t_VecChannels];
#pragma HLS ARRAY_PARTITION variable = l_vecIn complete dim = 0
    hls::stream<t_Interface> l_vecOut[t_VecChannels];
#pragma HLS ARRAY_PARTITION variable = l_vecOut complete dim = 0
    static_assert(HPC_maxVecSize % t_ParEntries == 0, "");
    xf::hpc::Buffer<HPC_biasInterface, HPC_maxVecSize / HPC_numActFuncs> l_biasBuffer;
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries> l_inVecBuffer[t_VecChannels];
#pragma HLS ARRAY_PARTITION variable = l_inVecBuffer complete dim = 1

#pragma HLS DATAFLOW
    loadWeights<t_Interface, t_ParEntries, t_WeightChannels>(p_batch, p_outVecSize, p_inVecSize, p_weightOffset,
                                                             p_As..., l_strA);
    l_inVecBuffer[0].readMem(p_inVecSize / t_ParEntries, p_vecIn0 + p_inputOffset, l_vecIn[0],
                             p_outVecSize / t_WeightChannels, p_batch);
#if HPC_vecChannels > 1
    l_inVecBuffer[1].readMem(p_inVecSize / t_ParEntries, p_vecIn1 + p_inputOffset, l_vecIn[1],
                             p_outVecSize / t_WeightChannels, p_batch);
#endif
#if HPC_vecChannels > 2
    l_inVecBuffer[2].readMem(p_inVecSize / t_ParEntries, p_vecIn2 + p_inputOffset, l_vecIn[2],
                             p_outVecSize / t_WeightChannels, p_batch);
#endif
#if HPC_vecChannels > 3
    l_inVecBuffer[3].readMem(p_inVecSize / t_ParEntries, p_vecIn3 + p_inputOffset, l_vecIn[3],
                             p_outVecSize / t_WeightChannels, p_batch);
#endif
    l_biasBuffer.readMem(p_outVecSize / HPC_numActFuncs, p_bias + p_biasOffset, l_bias, p_batch);

    xf::hpc::mlp::fcn<HPC_dataType, t_ParEntries, t_WeightChannels, t_VecChannels, HPC_numActFuncs>(
        p_batch * p_outVecSize / t_WeightChannels, p_inVecSize, p_act, l_strA, l_bias, l_vecIn, l_vecOut);

    stream2mem(p_outVecSize / t_ParEntries, l_vecOut[0], p_vecOut0 + p_outputOffset, p_batch);
#if HPC_vecChannels > 1
    stream2mem(p_outVecSize / t_ParEntries, l_vecOut[1], p_vecOut1 + p_outputOffset, p_batch);
#endif
#if HPC_vecChannels > 2
    stream2mem(p_outVecSize / t_ParEntries, l_vecOut[2], p_vecOut2 + p_outputOffset, p_batch);
#endif
#if HPC_vecChannels > 3
    stream2mem(p_outVecSize / t_ParEntries, l_vecOut[3], p_vecOut3 + p_outputOffset, p_batch);
#endif
}

template <typename... Ts>
void parseInstr(hls::stream<xf::hpc::Signal_t>& p_signal,
                hls::stream<xf::hpc::Clock_t>& p_clock,
                uint8_t* p_instr,
                Ts... p_As) {
    static_assert(0 == (8 * HPC_instrBytes) % HPC_wideType::t_TypeWidth, "");
    xf::hpc::mlp::FcnInstr<HPC_instrBytes> fcnInstr;
#ifdef __SYNTHESIS__
    p_signal.write(xf::hpc::START);
#endif
    for (int i = 0; i < HPC_maxInstrs; i++) {
        fcnInstr.template load<uint8_t>(p_instr + i * HPC_instrBytes);
        int l_outVecSize = fcnInstr.getOutVecSize();
        int l_inVecSize = fcnInstr.getInVecSize();
        int p_batch = fcnInstr.getBatch();
        int p_matrixOffset = fcnInstr.getWeightsOffset() * 8 / HPC_wideType::t_TypeWidth;
        int p_inputOffset = fcnInstr.getInputOffset() * 8 / HPC_wideType::t_TypeWidth;
        int p_biasOffset = fcnInstr.getBiasOffset() * 8 / HPC_biasType::t_TypeWidth;
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
        fcnInstr.template store<uint8_t>(p_instr + i * HPC_instrBytes);
    }

#ifdef __SYNTHESIS__
    p_signal.write(xf::hpc::STOP);
#endif
}
#endif

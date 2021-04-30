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
                   t_Interface* p_bias,
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
    hls::stream<t_Interface> l_vecIn[t_VecChannels], l_bias;
    hls::stream<t_Interface> l_vecOut[t_VecChannels];
#pragma HLS DATAFLOW
    loadWeights<t_Interface, t_ParEntries, t_WeightChannels>(p_batch, p_outVecSize, p_inVecSize, p_weightOffset,
                                                             p_As..., l_strA);
    static_assert(HPC_maxVecSize % t_ParEntries == 0, "");
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::readMem(
        p_inVecSize / t_ParEntries, p_vecIn0 + p_inputOffset, l_vecIn[0], p_outVecSize / t_WeightChannels, p_batch);
#if HPC_vecChannels > 1
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::readMem(
        p_inVecSize / t_ParEntries, p_vecIn1 + p_inputOffset, l_vecIn[1], p_outVecSize / t_WeightChannels, p_batch);
#endif
#if HPC_vecChannels > 2
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::readMem(
        p_inVecSize / t_ParEntries, p_vecIn2 + p_inputOffset, l_vecIn[2], p_outVecSize / t_WeightChannels, p_batch);
#endif
#if HPC_vecChannels > 3
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::readMem(
        p_inVecSize / t_ParEntries, p_vecIn3 + p_inputOffset, l_vecIn[3], p_outVecSize / t_WeightChannels, p_batch);
#endif

    xf::hpc::Buffer<t_Interface, HPC_maxVecSize / t_ParEntries>::readMem(p_outVecSize / t_ParEntries,
                                                                         p_bias + p_biasOffset, l_bias, p_batch);

    xf::hpc::mlp::fcn<HPC_dataType, t_ParEntries, t_WeightChannels, t_VecChannels, HPC_numActFuncs>(
        p_batch * p_outVecSize / t_WeightChannels, p_inVecSize, p_act, l_strA, l_bias, l_vecIn, l_vecOut);

    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::writeMem(
        p_outVecSize / t_ParEntries, l_vecOut[0], p_vecOut0 + p_outputOffset, p_batch);
#if HPC_vecChannels > 1
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::writeMem(
        p_outVecSize / t_ParEntries, l_vecOut[1], p_vecOut1 + p_outputOffset, p_batch);
#endif
#if HPC_vecChannels > 2
    xf::hpc::DoubleBuffer<t_Interface, HPC_maxVecSize / t_ParEntries>::writeMem(
        p_outVecSize / t_ParEntries, l_vecOut[2], p_vecOut2 + p_outputOffset, p_batch);
#endif
#if HPC_vecChannels > 3
    xf::blas::stream2mem<t_Interface>(p_outVecSize / t_ParEntries, l_vecOut[3], p_vecOut3 + p_outputOffset, p_batch);
#endif
}

#endif

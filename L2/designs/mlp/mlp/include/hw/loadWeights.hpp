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

#ifndef __HPC_LOADWEIGHTS_HPP__
#define __HPC_LOADWEIGHTS_HPP__
#include "buffer.hpp"

template <typename t_Interface, int t_ParEntries, int t_WeightChannels>
void loadWeights(uint32_t p_batch,
                 uint32_t p_outVecSize,
                 uint32_t p_inVecSize,
                 uint32_t p_weightOffset,
                 t_Interface* p_A0,
                 t_Interface* p_A1,
                 t_Interface* p_A2,
                 t_Interface* p_A3,
#if HPC_numChannels > 4
                 t_Interface* p_A4,
                 t_Interface* p_A5,
                 t_Interface* p_A6,
                 t_Interface* p_A7,
#endif
#if HPC_numChannels > 8
                 t_Interface* p_A8,
                 t_Interface* p_A9,
                 t_Interface* p_Aa,
                 t_Interface* p_Ab,
#endif
#if HPC_numChannels > 12
                 t_Interface* p_Ac,
                 t_Interface* p_Ad,
                 t_Interface* p_Ae,
                 t_Interface* p_Af,
#endif
#if HPC_numChannels > 16
                 t_Interface* p_A10,
                 t_Interface* p_A11,
                 t_Interface* p_A12,
                 t_Interface* p_A13,
#endif
#if HPC_numChannels > 20
                 t_Interface* p_A14,
                 t_Interface* p_A15,
                 t_Interface* p_A16,
                 t_Interface* p_A17,
#endif
#if HPC_numChannels > 24
                 t_Interface* p_A18,
#endif
#if HPC_numChannels > 25
                 t_Interface* p_A19,
                 t_Interface* p_A1a,
                 t_Interface* p_A1b,
#endif
#if HPC_numChannels > 28
                 t_Interface* p_A1c,
                 t_Interface* p_A1d,
                 t_Interface* p_A1e,
                 t_Interface* p_A1f,
#endif
                 hls::stream<t_Interface> l_strA[t_WeightChannels]) {
    constexpr int l_factor = t_ParEntries * t_WeightChannels;
    uint32_t l_mn = p_outVecSize * p_inVecSize / l_factor;
    constexpr int l_bufferSize = HPC_bufferSize / l_factor;
    static_assert(HPC_bufferSize % l_factor == 0, "");
#pragma HLS DATAFLOW
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A0 + p_weightOffset, l_strA[0x0], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1 + p_weightOffset, l_strA[0x1], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A2 + p_weightOffset, l_strA[0x2], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A3 + p_weightOffset, l_strA[0x3], p_batch);
#if HPC_numChannels > 4
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A4 + p_weightOffset, l_strA[0x4], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A5 + p_weightOffset, l_strA[0x5], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A6 + p_weightOffset, l_strA[0x6], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A7 + p_weightOffset, l_strA[0x7], p_batch);
#endif
#if HPC_numChannels > 8
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A8 + p_weightOffset, l_strA[0x8], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A9 + p_weightOffset, l_strA[0x9], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Aa + p_weightOffset, l_strA[0xa], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Ab + p_weightOffset, l_strA[0xb], p_batch);
#endif
#if HPC_numChannels > 12
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Ac + p_weightOffset, l_strA[0xc], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Ad + p_weightOffset, l_strA[0xd], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Ae + p_weightOffset, l_strA[0xe], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_Af + p_weightOffset, l_strA[0xf], p_batch);
#endif
#if HPC_numChannels > 16
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A10 + p_weightOffset, l_strA[0x10], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A11 + p_weightOffset, l_strA[0x11], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A12 + p_weightOffset, l_strA[0x12], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A13 + p_weightOffset, l_strA[0x13], p_batch);
#endif
#if HPC_numChannels > 20
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A14 + p_weightOffset, l_strA[0x14], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A15 + p_weightOffset, l_strA[0x15], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A16 + p_weightOffset, l_strA[0x16], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A17 + p_weightOffset, l_strA[0x17], p_batch);
#endif
#if HPC_numChannels > 24
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A18 + p_weightOffset, l_strA[0x18], p_batch);
#endif
#if HPC_numChannels > 25
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A19 + p_weightOffset, l_strA[0x19], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1a + p_weightOffset, l_strA[0x1a], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1b + p_weightOffset, l_strA[0x1b], p_batch);
#endif
#if HPC_numChannels > 28
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1c + p_weightOffset, l_strA[0x1c], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1d + p_weightOffset, l_strA[0x1d], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1e + p_weightOffset, l_strA[0x1e], p_batch);
    xf::hpc::Buffer<t_Interface, l_bufferSize>::readMem(l_mn, p_A1f + p_weightOffset, l_strA[0x1f], p_batch);
#endif
}

#endif

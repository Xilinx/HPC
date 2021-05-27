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
                 t_Interface* p_Ac,
                 t_Interface* p_Ad,
                 t_Interface* p_Ae,
                 t_Interface* p_Af,
#endif
                 hls::stream<t_Interface> l_strA[t_WeightChannels]) {
    constexpr int l_factor = t_ParEntries * t_WeightChannels;
    uint32_t l_mn = p_outVecSize * p_inVecSize / l_factor;
    constexpr int l_bufferSize = HPC_bufferSize / l_factor;
    static_assert(HPC_bufferSize % l_factor == 0, "");
    xf::hpc::Buffer<t_Interface, l_bufferSize> l_buffer[t_WeightChannels];
#pragma HLS ARRAY_PARTITION variable = l_buffer complete dim = 1
#pragma HLS DATAFLOW
    l_buffer[0x0].readMem(l_mn, p_A0 + p_weightOffset, l_strA[0x0], p_batch);
    l_buffer[0x1].readMem(l_mn, p_A1 + p_weightOffset, l_strA[0x1], p_batch);
    l_buffer[0x2].readMem(l_mn, p_A2 + p_weightOffset, l_strA[0x2], p_batch);
    l_buffer[0x3].readMem(l_mn, p_A3 + p_weightOffset, l_strA[0x3], p_batch);
#if HPC_numChannels > 4
    l_buffer[0x4].readMem(l_mn, p_A4 + p_weightOffset, l_strA[0x4], p_batch);
    l_buffer[0x5].readMem(l_mn, p_A5 + p_weightOffset, l_strA[0x5], p_batch);
    l_buffer[0x6].readMem(l_mn, p_A6 + p_weightOffset, l_strA[0x6], p_batch);
    l_buffer[0x7].readMem(l_mn, p_A7 + p_weightOffset, l_strA[0x7], p_batch);
#endif
#if HPC_numChannels > 8
    l_buffer[0x8].readMem(l_mn, p_A8 + p_weightOffset, l_strA[0x8], p_batch);
    l_buffer[0x9].readMem(l_mn, p_A9 + p_weightOffset, l_strA[0x9], p_batch);
    l_buffer[0xa].readMem(l_mn, p_Aa + p_weightOffset, l_strA[0xa], p_batch);
    l_buffer[0xb].readMem(l_mn, p_Ab + p_weightOffset, l_strA[0xb], p_batch);
    l_buffer[0xc].readMem(l_mn, p_Ac + p_weightOffset, l_strA[0xc], p_batch);
    l_buffer[0xd].readMem(l_mn, p_Ad + p_weightOffset, l_strA[0xd], p_batch);
    l_buffer[0xe].readMem(l_mn, p_Ae + p_weightOffset, l_strA[0xe], p_batch);
    l_buffer[0xf].readMem(l_mn, p_Af + p_weightOffset, l_strA[0xf], p_batch);
#endif
}

#endif

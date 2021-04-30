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

#include "xf_blas.hpp"
#ifndef XF_HPC_KERNEL_GEMVMULT_HPP
#define XF_HPC_KERNEL_GEMVMULT_HPP

typedef xf::blas::WideType<HPC_dataType, HPC_parEntries> HPC_wideType;
typedef HPC_wideType::t_TypeInt HPC_interface;

/**
 * @brief krnl_fcn kernel function to compute A * p
 *
 * @param p_m the row size of matrix A
 * @param p_n the col size of matrix A
 * @param p_A[0..f] the memory ports to vector A
 * @param p_vecIn the input memory address to vector pk
 * @param p_vecOut the output memory address to vector Apk
 *
 */

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
                         HPC_interface* p_bias);
#endif

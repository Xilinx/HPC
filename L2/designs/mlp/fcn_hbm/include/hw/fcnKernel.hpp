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
#ifndef XF_HPC_KERNEL_FCN_HPP
#define XF_HPC_KERNEL_FCN_HPP

typedef xf::blas::WideType<HPC_dataType, HPC_parEntries> HPC_wideType;
typedef typename HPC_wideType::t_TypeInt HPC_interface;

typedef xf::blas::WideType<HPC_dataType, HPC_numActFuncs> HPC_biasType;
typedef HPC_biasType::t_TypeInt HPC_biasInterface;

/**
 * @brief krnl_fcn kernel function to compute y = f(W * x + b)
 *
 * @param p_m the row size of matrix W
 * @param p_n the col size of matrix W
 * @param p_W[0..f] the memory ports to vector W
 * @param p_vecIn the input memory address to vector pk
 * @param p_vecOut the output memory address to vector Apk
 *
 */
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
                         HPC_biasInterface* p_bias);
#endif

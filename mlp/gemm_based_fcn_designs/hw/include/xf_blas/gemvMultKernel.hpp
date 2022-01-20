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
#ifndef XF_BLAS_KERNEL_GEMVMULT_HPP
#define XF_BLAS_KERNEL_GEMVMULT_HPP

typedef xf::blas::WideType<BLAS_dataType, BLAS_parEntries> BLAS_wideType;
typedef BLAS_wideType::t_TypeInt BLAS_interface;

/**
 * @brief krnl_gemv kernel function to compute A * p
 *
 * @param p_m the row size of matrix A
 * @param p_n the col size of matrix A
 * @param p_A[0..f] the memory ports to vector A
 * @param p_pk the input memory address to vector pk
 * @param p_Apk the output memory address to vector Apk
 *
 */

extern "C" void krnl_gemv(uint32_t p_m,
                          uint32_t p_n,
                          BLAS_interface* p_A0,
#if BLAS_numChannels > 1
                          BLAS_interface* p_A1,
#endif
#if BLAS_numChannels > 2
                          BLAS_interface* p_A2,
                          BLAS_interface* p_A3,
#endif
#if BLAS_numChannels > 4
                          BLAS_interface* p_A4,
                          BLAS_interface* p_A5,
                          BLAS_interface* p_A6,
                          BLAS_interface* p_A7,
#endif
#if BLAS_numChannels > 8
                          BLAS_interface* p_A8,
                          BLAS_interface* p_A9,
                          BLAS_interface* p_Aa,
                          BLAS_interface* p_Ab,
                          BLAS_interface* p_Ac,
                          BLAS_interface* p_Ad,
                          BLAS_interface* p_Ae,
                          BLAS_interface* p_Af,
#endif
                          BLAS_interface* p_pk,
                          BLAS_interface* p_Apk);

#endif

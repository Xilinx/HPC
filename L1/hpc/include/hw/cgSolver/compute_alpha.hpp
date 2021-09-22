/*
 * Copyright 2019-2021 Xilinx, Inc.
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
#include "token.hpp"
#include "xf_blas.hpp"
#include "streamOps.hpp"
#include "nrm2s.hpp"

#ifndef XF_HPC_CG_ALPHA_HPP
#define XF_HPC_CG_ALPHA_HPP

namespace xf {
namespace hpc {
namespace cg {

template <typename t_DataType, int t_ParEntries>
void proc_compuate_alpha(uint32_t p_size, 
        typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt* p_pk,
        typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt* p_Apk,
        t_DataType &p_dot) {
    typedef typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt t_TypeInt;
#pragma HLS DATAFLOW
    hls::stream<t_TypeInt> l_pkStrIn, l_ApkStrIn;
    xf::blas::mem2stream(p_size / t_ParEntries, p_pk, l_pkStrIn);
    xf::blas::mem2stream(p_size / t_ParEntries, p_Apk, l_ApkStrIn);
    xf::blas::dot<t_DataType, xf::blas::mylog2(CG_parEntries)>(p_size, l_pkStrIn, l_ApkStrIn, p_dot);
}

template <typename t_DataType, int t_ParEntries, int t_TkWidth = 8>
void compute_alpha(typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt* p_pk,
               typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt* p_Apk,
               hls::stream<ap_uint<t_TkWidth> >& p_tokenIn,
               hls::stream<ap_uint<t_TkWidth> >& p_tokenOut) {
    Token<t_DataType> l_token;
    StreamInstr<sizeof(l_token)> l_cs;
    l_token.read_decode(p_tokenIn, l_cs);

    while (!l_token.getExit()) {
        uint32_t l_size = l_token.getVecSize();
        t_DataType l_dot, rz = l_token.getRZ();
        proc_compuate_alpha<t_DataType, t_ParEntries>(l_size, p_pk, p_Apk, l_dot);
        ap_wait();
        l_token.setAlpha(rz / l_dot);
        l_token.encode_write(p_tokenOut, l_cs);
        l_token.read_decode(p_tokenIn, l_cs);
    }
    l_token.encode_write(p_tokenOut, l_cs);
}
}
}
}
#endif


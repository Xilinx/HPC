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
#include "streamOps.hpp"
#include "activations.hpp"
#include "fcnInstr.hpp"
#ifndef XF_HPC_KERNEL_GEMV_HPP
#define XF_HPC_KERNEL_GEMV_HPP

namespace xf {
namespace hpc {
namespace mlp {

template <typename t_DataType,
          unsigned int t_ParEntries,
          typename T0 = typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt>
void streamActivation(const unsigned int p_n, hls::stream<T0>& p_in, hls::stream<T0>& p_out, const ActFunc_t p_act) {
    switch (p_act) {
        case ActFunc_t::RELU:
            for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
                xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
                xf::blas::WideType<t_DataType, t_ParEntries> o;
                for (int j = 0; j < t_ParEntries; j++) {
                    o[j] = relu(r[j]);
                }
                p_out.write(o);
            }
            break;
        case ActFunc_t::SIGMOID:
            for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
                xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
                xf::blas::WideType<t_DataType, t_ParEntries> o;
                for (int j = 0; j < t_ParEntries; j++) {
                    o[j] = sigmoid(r[j]);
                }
                p_out.write(o);
            }
            break;
        /*
    case ActFunc_t::TANSIG:
        for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
            xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
            xf::blas::WideType<t_DataType, t_ParEntries> o;
            for (int j = 0; j < t_ParEntries; j++) {
                o[j] = tansig(r[j]);
            }
            p_out.write(o);
        }
        break;
        */
        default:
            for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
                p_out.write(p_in.read());
            }
            break;
    }
}

template <typename t_DataType,
          int t_ParEntries,
          int t_WeightChannels,
          int t_VecChannels,
          int t_NumActFuncs,
          typename T0 = typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt>
void fcn(uint32_t p_m,
         uint32_t p_n,
         const ActFunc_t p_act,
         hls::stream<T0> p_W[t_WeightChannels],
         hls::stream<T0>& p_bias,
         hls::stream<T0> p_vecIn[t_VecChannels],
         hls::stream<T0> p_vecOut[t_VecChannels]) {
    static_assert(0 == t_ParEntries % t_NumActFuncs, "");
    static_assert(0 == t_WeightChannels % t_NumActFuncs, "");

    typedef typename xf::blas::WideType<t_DataType, 1>::t_TypeInt T1;
    typedef typename xf::blas::WideType<t_DataType, t_NumActFuncs>::t_TypeInt T2;
    constexpr int t_LogParEntries = xf::blas::mylog2(t_ParEntries);

    hls::stream<T0> l_pk[t_VecChannels][t_WeightChannels];
#pragma HLS ARRAY_PARTITION variable = l_pk complete dim = 0
#pragma HLS stream variable = l_pk depth = 64

    hls::stream<T0> l_W[t_WeightChannels][t_VecChannels];
#pragma HLS ARRAY_PARTITION variable = l_W complete dim = 0

    hls::stream<T1> l_strR[t_VecChannels][t_WeightChannels];
#pragma HLS ARRAY_PARTITION variable = l_strR complete dim = 0
#pragma HLS stream variable = l_strR depth = 64

    hls::stream<T2> l_strC[t_VecChannels], l_strAdd[t_VecChannels], l_strAct[t_VecChannels], l_bias[t_VecChannels],
        l_strb;
#pragma HLS ARRAY_PARTITION variable = l_bias complete dim = 1
#pragma HLS ARRAY_PARTITION variable = l_strC complete dim = 1

#pragma HLS DATAFLOW
    wide2stream<sizeof(t_DataType) * 8 * t_NumActFuncs, t_ParEntries / t_NumActFuncs>(
        p_m * t_WeightChannels / t_ParEntries, p_bias, l_strb);
    streamFwd<t_VecChannels>(p_m * t_WeightChannels / t_NumActFuncs, l_strb, l_bias);
    for (int i = 0; i < t_VecChannels; i++) {
#pragma HLS UNROLL
        streamFwd<t_WeightChannels>(p_m * p_n / t_ParEntries, p_vecIn[i], l_pk[i]);
    }

    for (int i = 0; i < t_WeightChannels; i++) {
#pragma HLS UNROLL
        streamFwd<t_VecChannels>(p_m * p_n / t_ParEntries, p_W[i], l_W[i]);
        for (int j = 0; j < t_VecChannels; j++) {
#pragma HLS UNROLL
            xf::blas::gemv<t_DataType, t_LogParEntries>(p_m, p_n, l_W[i][j], l_pk[j][i], l_strR[j][i]);
        }
    }

    for (int i = 0; i < t_VecChannels; i++) {
#pragma HLS UNROLL
        collectStream<t_DataType, t_WeightChannels, t_NumActFuncs>(p_m, l_strR[i], l_strC[i]);
        blas::axpy<t_DataType, t_NumActFuncs>(p_m * t_WeightChannels, 1, l_strC[i], l_bias[i], l_strAdd[i]);
        streamActivation<t_DataType, t_NumActFuncs>(p_m * t_WeightChannels / t_NumActFuncs, l_strAdd[i], l_strAct[i],
                                                    p_act);
        stream2wide<sizeof(t_DataType) * 8 * t_NumActFuncs, t_ParEntries / t_NumActFuncs>(
            p_m * t_WeightChannels / t_ParEntries, l_strAct[i], p_vecOut[i]);
    }
}
}
}
}
#endif

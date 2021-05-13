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
#include "activations.hpp"
#include "fcnInstr.hpp"
#ifndef XF_HPC_STREAM_ACT_HPP
#define XF_HPC_STREAM_ACT_HPP

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
}
}
}
#endif

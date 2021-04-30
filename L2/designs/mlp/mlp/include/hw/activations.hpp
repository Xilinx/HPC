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

#include "hls_stream.h"
#include "xf_blas.hpp"
#include "fcnInstr.hpp"

using namespace xf::blas;

#ifndef XF_HPC_MLP_ACTIVATIONS_HPP
#define XF_HPC_MLP_ACTIVATIONS_HPP

/**
 * @file activations.hpp
 * @brief activation functions used in MLP
 * streaming modules for 2D-MLP2D kernels
 */

namespace xf {
namespace hpc {
namespace mlp {

/** @brief linear, linear activation function, y = a * x + b
 *
 * @param x the input value
 * @param a the linear factor
 * @param b the bias
 */
template <typename t_DataType>
t_DataType linear(t_DataType x, t_DataType a = 1, t_DataType b = 0) {
    return a * x + b;
}

/** @brief identity, special linear activation function, y = x
 *
 * @param x the input value
 */
template <typename t_DataType>
t_DataType identity(t_DataType x) {
    return x;
}

/** @brief relu (rectified linear unit) is a very common activation function in
 * deep neural network
 *
 * @param x the input value
 */
template <typename t_DataType>
t_DataType relu(t_DataType x) {
    if (x > 0)
        return x;
    else
        return 0;
}

/** @brief sigmoid function is a very common activation function in MLP
 *
 * @param x is the input value
 */
template <typename t_DataType>
t_DataType sigmoid(t_DataType x) {
    t_DataType l_exp = hls::expf(-x);
    return 1.0f / (1.0f + l_exp);
}

/** @brief tansig function is used as an activation function in some MLPs
 *
 * @param x is the input value
 */
template <typename t_DataType>
t_DataType tansig(t_DataType x) {
    t_DataType l_exp = hls::expf(-2.0f * x);
    return 2.0f / (1.0f + l_exp) - 1.0f;
}

template <typename t_DataType,
          unsigned int t_ParEntries,
          typename T0 = typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt>
void streamActivation(const unsigned int p_n, hls::stream<T0>& p_in, hls::stream<T0>& p_out, const ActFunc_t p_act) {
    switch (p_act) {
        case ActFunc_t::LINEAR:
            for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
                p_out.write(p_in.read());
            }
            break;
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
                xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
                xf::blas::WideType<t_DataType, t_ParEntries> o;
                for (int j = 0; j < t_ParEntries; j++) {
                    o[j] = r[j];
                }
                p_out.write(o);
            }
            break;
    }
}

template <typename t_DataType>
void streamLinear(unsigned int p_n, hls::stream<t_DataType>& p_in, hls::stream<t_DataType>& p_out) {
    for (int i = 0; i < p_n; i++) {
        p_out.write(linear(p_in.read()));
    }
}

template <typename t_DataType,
          unsigned int t_ParEntries = 1,
          typename T0 = typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt>
void streamRelu(unsigned int p_n, hls::stream<T0>& p_in, hls::stream<T0>& p_out) {
    for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
        xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
        xf::blas::WideType<t_DataType, t_ParEntries> o;
        for (int j = 0; j < t_ParEntries; j++) {
            o[j] = relu(r[j]);
        }
        p_out.write(o);
    }
}

template <typename t_DataType,
          unsigned int t_ParEntries = 1,
          typename T0 = typename xf::blas::WideType<t_DataType, t_ParEntries>::t_TypeInt>
void streamSigmoid(unsigned int p_n, hls::stream<T0>& p_in, hls::stream<T0>& p_out) {
    for (int i = 0; i < p_n; i++) {
#pragma HLS PIPELINE
        xf::blas::WideType<t_DataType, t_ParEntries> r = p_in.read();
        xf::blas::WideType<t_DataType, t_ParEntries> o;
        for (int j = 0; j < t_ParEntries; j++) {
            o[j] = sigmoid(r[j]);
        }
        p_out.write(o);
    }
}

} // end namespace mlp
} // end namespace hpc
} // end namespace xf
#endif

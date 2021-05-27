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
#ifndef XF_HPC_MLP_ACTIVATIONS_HPP
#define XF_HPC_MLP_ACTIVATIONS_HPP
#include <cmath>
#include "fcnInstr.hpp"
namespace xf {
namespace hpc {
namespace mlp {

template <typename T>
T softmax(uint32_t p_batch, uint32_t p_inSize, T* data) {
    for (int b = 0; b < p_batch; b++) {
        T sum = 0;
        T* ex = new T[p_inSize];
        for (int i = 0; i < p_inSize; i++) {
            ex[i] = std::exp(data[b * p_inSize + i]);
            sum += ex[i];
        }
        for (int i = 0; i < p_inSize; i++) {
            data[b * p_inSize + i] = ex[i] / sum;
        }
        delete[] ex;
    }
}

template <typename T>
T sigmoid(T x) {
    T p = std::exp(-x);
    return T(1.0) / (T(1.0) + p);
}

template <typename T>
T relu(T x) {
    return std::max(x, T(0));
}

template <typename T>
void copyBias(uint32_t batch, uint32_t dim, T* x, T* bias) {
    for (int i = 0; i < batch; i++) {
        for (int j = 0; j < dim; j++) x[i * dim + j] = bias[j];
    }
}

template <typename T>
void funcBatchAct(uint32_t batch, uint32_t dim, T* x, ActFunc_t act) {
    switch (act) {
        case ActFunc_t::RELU:
            for (int i = 0; i < batch * dim; i++) {
                x[i] = relu(x[i]);
            }
            break;
        case ActFunc_t::SIGMOID:
            for (int i = 0; i < batch * dim; i++) {
                x[i] = sigmoid(x[i]);
            }
            break;
        case ActFunc_t::SOFTMAX:
            softmax(batch, dim, x);
            break;
        default:
            return;
            break;
    }
}
}
}
}
#endif

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
#ifndef __XF_HPC_MLP_HPP__
#define __XF_HPC_MLP_HPP__
#include <cstdint>
#include <vector>
#include <cstdlib>
#include "fcnInstr.hpp"
#include "binFiles.hpp"
#include "fpga.hpp"
using namespace std;

namespace xf {
namespace hpc {
namespace mlp {
template <typename T>
class FCN {
   public:
    int m_InputSize;
    int m_OutputSize;
    host_buffer_t<T> m_Weight;
    host_buffer_t<T> m_Bias;
    ActFunc_t m_ActFunc;
    bool m_Allocated = false;

   public:
    FCN() { m_ActFunc = ActFunc_t::LINEAR; }
    ~FCN() {
        if (m_Allocated) {
        }
    }
    void setDim(int m_InputSize, int m_OutputSize) {
        this->m_InputSize = m_InputSize;
        this->m_OutputSize = m_OutputSize;
        m_Bias.resize(m_OutputSize);
        m_Weight.resize(m_InputSize * m_OutputSize);
    }

    void setActFunc(ActFunc_t ActFunc) { this->m_ActFunc = ActFunc; }

    void loadData(string path_to_wi, string path_to_bi) {
        readBin(path_to_wi, m_InputSize * m_OutputSize * sizeof(T), m_Weight);
        readBin(path_to_bi, m_OutputSize * sizeof(T), m_Bias);
    }
    void setData(T* weight, T* bias) {
        copy(weight, weight + m_InputSize * m_OutputSize, m_Weight.begin());
        copy(bias, bias + m_OutputSize, m_Bias.begin());
    }
    void setData(host_buffer_t<T>& weight, host_buffer_t<T>& bias) {
        copy(weight.begin(), weight.end(), m_Weight.begin());
        copy(bias.begin(), bias.end(), m_Bias.begin());
    }
};

template <typename T>
class MLP {
   public:
    int32_t m_NumLayers = 0;
    vector<uint32_t> m_Dims;
    vector<FCN<T> > m_Layers;

   public:
    MLP(int num) {
        m_NumLayers = num;
        m_Dims.resize(num + 1);
        m_Layers.resize(num);
    }

    void setDim(const uint32_t* m_Dims) {
        copy(m_Dims, m_Dims + m_NumLayers + 1, this->m_Dims.begin());
        for (int i = 0; i < m_NumLayers; i++) {
            m_Layers[i].setDim(m_Dims[i], m_Dims[i + 1]);
        }
    }

    void setActFunc(int layerID, ActFunc_t ActFunc) { m_Layers[layerID].setActFunc(ActFunc); }

    void setActFunc(ActFunc_t ActFunc) {
        for (int i = 0; i < m_NumLayers - 1; i++) {
            m_Layers[i].setActFunc(ActFunc);
        }
    }

    void loadLayer(string filePath) {
        for (int id = 0; id < m_NumLayers; id++) {
            string m_WeightPath = filePath + "W_" + to_string(id) + ".mat";
            string m_BiasPath = filePath + "b_" + to_string(id) + ".mat";
            m_Layers[id].loadData(m_WeightPath, m_BiasPath);
        }
    }

    void setLayer(T** weights, T** bias) {
        for (int i = 0; i < m_NumLayers; i++) m_Layers[i].setData(weights[i], bias[i]);
    }

    void setLayer(int layerId, T* weights, T* bias) { m_Layers[layerId].setData(weights, bias); }
};
}
}
}
#endif

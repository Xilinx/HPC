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
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include "fcnInstr.hpp"
#include "binFiles.hpp"
#include "fpga.hpp"
#include "activations.hpp"
#ifdef MKLROOT
#include <mkl.h>
#endif
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
    }

    void setActFunc(ActFunc_t ActFunc) { this->m_ActFunc = ActFunc; }

    void loadData(string path_to_wi, string path_to_bi) {
        readBin(path_to_wi, m_InputSize * m_OutputSize * sizeof(T), m_Weight);
        readBin(path_to_bi, m_OutputSize * sizeof(T), m_Bias);
    }
    void setData(T* weight, T* bias) {
        m_Bias.resize(m_OutputSize);
        m_Weight.resize(m_InputSize * m_OutputSize);
        copy(weight, weight + m_InputSize * m_OutputSize, m_Weight.begin());
        copy(bias, bias + m_OutputSize, m_Bias.begin());
    }
    void setData(host_buffer_t<T>& weight, host_buffer_t<T>& bias) {
        m_Bias.resize(m_OutputSize);
        m_Weight.resize(m_InputSize * m_OutputSize);
        copy(weight.begin(), weight.end(), m_Weight.begin());
        copy(bias.begin(), bias.end(), m_Bias.begin());
    }

    void inference(uint32_t batch, T* vecIn, T* vecOut) {
#ifdef MKLROOT
        copyBias(batch, m_OutputSize, vecOut, m_Bias.data());
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, batch, m_OutputSize, m_InputSize, 1.0f, vecIn, m_InputSize,
                    m_Weight.data(), m_InputSize, 1.0f, vecOut, m_OutputSize);
#else
#pragma omp parallel for
        for (int i = 0; i < batch; i++) {
            for (int j = 0; j < m_OutputSize; j++) {
                T sum = m_Bias[j];
                for (int k = 0; k < m_InputSize; k++) sum += vecIn[i * m_InputSize + k] * m_Weight[j * m_InputSize + k];
                vecOut[i * m_OutputSize + j] = sum;
            }
        }
#endif
        funcBatchAct(batch, m_OutputSize, vecOut, m_ActFunc);
    }
};

template <typename T>
class MLP {
   public:
    uint32_t m_NumLayers = 0;
    uint32_t m_MaxVecDim = 0;
    vector<uint32_t> m_Dims;
    vector<FCN<T> > m_Layers;

   public:
    MLP(int num) {
        m_NumLayers = num;
        m_Dims.resize(num + 1);
        m_Layers.resize(num);
    }

    void setDim(vector<uint32_t>& m_Dims) {
        assert(m_Dims.size() == m_NumLayers + 1);
        setDim(m_Dims.data());
    }

    void setDim(const uint32_t* m_Dims) {
        copy(m_Dims, m_Dims + m_NumLayers + 1, this->m_Dims.begin());
        for (int i = 0; i < m_NumLayers; i++) {
            m_Layers[i].setDim(m_Dims[i], m_Dims[i + 1]);
        }
        m_MaxVecDim = *max_element(this->m_Dims.begin() + 1, this->m_Dims.end());
    }

    void setActFunc(int layerID, ActFunc_t ActFunc) { m_Layers[layerID].setActFunc(ActFunc); }

    void setLastActFunc(ActFunc_t ActFunc) { m_Layers.back().setActFunc(ActFunc); }
    void setActFunc(ActFunc_t ActFunc) {
        for (int i = 0; i < m_NumLayers - 1; i++) {
            m_Layers[i].setActFunc(ActFunc);
        }
    }

    void loadLayer(string filePath) {
        for (int id = 0; id < m_NumLayers; id++) {
            string m_WeightPath = filePath + "/W_" + to_string(id) + ".mat";
            string m_BiasPath = filePath + "/b_" + to_string(id) + ".mat";
            m_Layers[id].loadData(m_WeightPath, m_BiasPath);
        }
    }

    void setLayer(T** weights, T** bias) {
        for (int i = 0; i < m_NumLayers; i++) m_Layers[i].setData(weights[i], bias[i]);
    }

    void setLayer(int layerId, T* weights, T* bias) { m_Layers[layerId].setData(weights, bias); }

    double inference(vector<T>& vecIn, vector<T>& vecOut) {
        uint32_t batch = vecIn.size() / m_Dims.front();
        if (vecOut.size() < batch * m_Dims.back()) vecOut.resize(batch * m_Dims.back());
        return inference(batch, vecIn.data(), vecOut.data());
    }
    double inference(uint32_t batch, T* vecIn, T* vecOut) {
        auto startTime = chrono::high_resolution_clock::now();
        if (m_NumLayers == 1) {
            m_Layers.front().inference(batch, vecIn, vecOut);
        } else {
            T* tmp[] = {new T[batch * m_MaxVecDim], new T[batch * m_MaxVecDim]};
            m_Layers.front().inference(batch, vecIn, tmp[0]);
            for (int i = 1; i < m_NumLayers - 1; i++) m_Layers[i].inference(batch, tmp[(i + 1) % 2], tmp[i % 2]);
            m_Layers.back().inference(batch, tmp[m_NumLayers % 2], vecOut);
            delete tmp[0];
            delete tmp[1];
        }
        auto finishTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();
        return t_sec;
    }
};
}
}
}
#endif

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
#ifndef __XF_HPC_MLP_KERNEL_HPP__
#define __XF_HPC_MLP_KERNEL_HPP__
#include <cstdint>
#include <vector>
#include <algorithm>
#include <chrono>
#include "fcnInstr.hpp"
#include "mlp.hpp"
#include "fpga.hpp"
#include "binFiles.hpp"
using namespace std;
using namespace xf::hpc::mlp;

template <typename t_DataType, int t_InstrBytes>
class MLPKernel : public Kernel {
   private:
    vector<host_buffer_t<t_DataType> > h_weights;
    vector<host_buffer_t<t_DataType> > h_vector;
    host_buffer_t<t_DataType> h_bias;
    host_buffer_t<uint8_t> h_instr;
    vector<FcnInstr<t_InstrBytes> > h_fcnInstr;

    vector<cl::Buffer> m_buffer_W;
    vector<cl::Buffer> m_buffer_v;
    vector<cl::Buffer> m_buffer_x;
    vector<cl::Buffer> m_buffer_y;
    cl::Buffer m_buffer_bias;
    cl::Buffer m_buffer_instr;

    int m_Batch = 0;
    int m_MaxVecDim = 0;
    int outputOffset = 0;
    MLP<t_DataType>* mlp = nullptr;

    const int m_VecChannels = 0;
    const int m_NumChannels = 0;
    const int m_ParEntries = 0;

   public:
    MLPKernel() {}
    MLPKernel(int weightChannels, int vecChannels, int parEntries)
        : m_VecChannels(vecChannels), m_NumChannels(weightChannels), m_ParEntries(parEntries) {}
    MLPKernel(FPGA* fpga, int weightChannels, int vecChannels, int parEntries)
        : Kernel(fpga), m_VecChannels(vecChannels), m_NumChannels(weightChannels), m_ParEntries(parEntries) {}

    void loadModel(MLP<t_DataType>* mlp) {
        h_weights.resize(m_NumChannels);
        h_vector.resize(m_VecChannels);
        this->mlp = mlp;
        int m_NumLayers = mlp->m_NumLayers;
        h_fcnInstr.resize(m_NumLayers);
        m_MaxVecDim = *max_element(mlp->m_Dims.begin() + 1, mlp->m_Dims.end());

        createWeightsBuffer();
        createVecBuffer();
        setModelArgs();

        int weightOffset = 0;
        int biasOffset = 0;

        for (int i = 0; i < mlp->m_NumLayers; i++) {
            h_fcnInstr[i].setOutVecSize(mlp->m_Dims[i + 1]);
            h_fcnInstr[i].setInVecSize(mlp->m_Dims[i]);
            h_fcnInstr[i].setBiasOffset(biasOffset);
            biasOffset += mlp->m_Dims[i + 1] * sizeof(t_DataType);
            h_fcnInstr[i].setWeightsOffset(weightOffset);
            weightOffset += mlp->m_Dims[i + 1] * mlp->m_Dims[i] * sizeof(t_DataType) / m_NumChannels;
            h_fcnInstr[i].setActivation(mlp->m_Layers[i].m_ActFunc);
        }
    }

    double inference(host_buffer_t<t_DataType>& h_x, host_buffer_t<t_DataType>& h_v) {
        setInput(h_x);
        auto startTime = chrono::high_resolution_clock::now();
        enqueueTask();
        finish();
        auto finishTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();
        getOutput(h_v.data());
        uint64_t l_last = 0;
        for (int i = 0; i < mlp->m_NumLayers + 1; i++) {
            xf::hpc::mlp::FcnInstr<t_InstrBytes> fcnInstr;
            fcnInstr.template load<uint8_t>(h_instr.data() + t_InstrBytes * i);
            uint64_t l_clock = fcnInstr.getClock();
            if (l_clock == 0) break;
            l_clock -= l_last;
            l_last += l_clock;
            cout << "Instruction " << i << ": ";
            cout << "HW measured time " << l_clock * HW_CLK << " seconds, ";
            cout << "HW efficiency: "
                 << 100.0 * fcnInstr.getOutVecSize() * fcnInstr.getInVecSize() * fcnInstr.getBatch() / m_ParEntries /
                        m_NumChannels / l_clock
                 << "%." << endl;
        }
        cout << "HW measured execution time " << l_last * HW_CLK << " seconds, ";
        return t_sec;
    }

    static double inference(vector<MLPKernel*> kernels,
                            host_buffer_t<t_DataType>& h_x,
                            host_buffer_t<t_DataType>& h_v) {
        uint32_t l_numK = kernels.size();
#pragma omp prallel for
        for (int i = 0; i < l_numK; i++) {
            kernels[i]->setInput(h_x.data() + i * h_x.size() / l_numK, h_x.size() / l_numK);
        }

        auto startTime = chrono::high_resolution_clock::now();
        for (auto kernel : kernels) kernel->enqueueTask();
        for (auto kernel : kernels) kernel->finish();
        auto finishTime = chrono::high_resolution_clock::now();
        for (int i = 0; i < l_numK; i++)
            kernels[i]->getOutput(h_v.data() + i * kernels[i]->m_Batch * kernels[i]->mlp->m_Dims.back());
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();

        return t_sec;
    }

   private:
    void createWeightsBuffer() {
        int biasSize = 0, weightSize = 0;
        for (auto& fcn : mlp->m_Layers) {
            assert(fcn.m_OutputSize % m_NumChannels == 0);
            assert(fcn.m_InputSize % m_ParEntries == 0);
            biasSize += fcn.m_OutputSize;
            weightSize += fcn.m_OutputSize * fcn.m_InputSize;
        }

        h_bias.reserve(biasSize);
        for (int i = 0; i < m_NumChannels; i++) {
            h_weights[i].reserve(weightSize / m_NumChannels);
        }

        for (auto& fcn : mlp->m_Layers) {
            h_bias.insert(h_bias.end(), fcn.m_Bias, fcn.m_Bias + fcn.m_OutputSize);
            for (int row = 0; row < fcn.m_OutputSize; row++)
                h_weights[row % m_NumChannels].insert(h_weights[row % m_NumChannels].end(),
                                                      fcn.m_Weight + row * fcn.m_InputSize,
                                                      fcn.m_Weight + (row + 1) * fcn.m_InputSize);
        }

        for (int i = 0; i < m_NumChannels; i++) {
            m_buffer_W.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_weights[i]));
        }
        m_buffer_bias = createDeviceBuffer(CL_MEM_READ_ONLY, h_bias);
    }
    void createVecBuffer() {
        for (int i = 0; i < m_VecChannels; i++) {
            h_vector[i].resize(256 * 1024 * 1024 / sizeof(t_DataType));
            m_buffer_v.push_back(createDeviceBuffer(CL_MEM_READ_WRITE, h_vector[i]));
        }
    }
    void setInput(t_DataType* p_x, uint32_t p_size) {
        assert(p_size % mlp->m_Dims.front() == 0);
        m_Batch = p_size / mlp->m_Dims.front();
        assert(m_Batch % m_VecChannels == 0);

        outputOffset = (2 * m_MaxVecDim + mlp->m_Dims.front()) * m_Batch / m_VecChannels;
        int outputSize = mlp->m_Dims.back() * m_Batch / m_VecChannels;
        int bufferSize = outputOffset + outputSize;
        assert(bufferSize * sizeof(t_DataType) <= 256 * 1024 * 1024);

        for (int i = 0; i < m_VecChannels; i++) {
            //    h_x[i].resize(bufferSize);
            copy(p_x + i * p_size / m_VecChannels, p_x + (i + 1) * p_size / m_VecChannels, h_vector[i].begin());
            cl_buffer_region buffer_info_x = {0, p_size / m_VecChannels * sizeof(t_DataType)};
            m_buffer_x.push_back(
                m_buffer_v[i].createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &buffer_info_x));
            cl_buffer_region buffer_info = {outputOffset * sizeof(t_DataType), outputSize * sizeof(t_DataType)};
            m_buffer_y.push_back(
                m_buffer_v[i].createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &buffer_info));
        }

        int outputOffset[2] = {mlp->m_Dims.front() * m_Batch / m_VecChannels * sizeof(t_DataType),
                               (m_MaxVecDim + mlp->m_Dims.front()) * m_Batch * sizeof(t_DataType) / m_VecChannels};

        h_instr.resize(t_InstrBytes * (1 + mlp->m_NumLayers));
        for (int i = 0; i < mlp->m_NumLayers; i++) {
            h_fcnInstr[i].setBatch(m_Batch / m_VecChannels);
            if (i == 0) {
                h_fcnInstr[i].setInputOffset(0);
            } else
                h_fcnInstr[i].setInputOffset(outputOffset[(i + 1) % 2]);
            if (i == mlp->m_NumLayers - 1)
                h_fcnInstr[i].setOutputOffset(this->outputOffset * sizeof(t_DataType));
            else
                h_fcnInstr[i].setOutputOffset(outputOffset[i % 2]);
            h_fcnInstr[i].template store<uint8_t>(h_instr.data() + i * t_InstrBytes);
        }
        setRunArgs();
    }

    void setInput(host_buffer_t<t_DataType>& p_x) { setInput(p_x.data(), p_x.size()); }

    void setModelArgs() {
        cl_int err;
        vector<cl::Memory> l_buffers;

        l_buffers.push_back(m_buffer_bias);
        for (auto x : m_buffer_W) l_buffers.push_back(x);
        for (auto x : m_buffer_v) l_buffers.push_back(x);

        // Setting Kernel Arguments
        uint32_t n_arg = 1;

        for (int i = 0; i < m_NumChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_W[i]));
        }
        for (int i = 0; i < m_VecChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_v[i]));
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_v[i]));
        }
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_bias));

        // Copy input data to device global memory
        sendBuffer(l_buffers); /* 0 means from host*/
        finish();
    }

    void setRunArgs() {
        cl_int err;
        vector<cl::Memory> l_buffers;

        m_buffer_instr = createDeviceBuffer(CL_MEM_READ_WRITE, h_instr);
        for (auto x : m_buffer_x) l_buffers.push_back(x);
        // Setting Kernel Arguments
        uint32_t n_arg = 0;
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_instr));

        // Copy input data to device global memory
        sendBuffer(l_buffers); /* 0 means from host*/
        finish();
    }

    void getOutput(t_DataType* h_v) {
        vector<cl::Memory> h_m;
        h_m.push_back(m_buffer_instr);
        for (auto& y : m_buffer_y) h_m.push_back(y);
        getBuffer(h_m);
        for (int i = 0; i < m_VecChannels; i++) {
            copy(h_vector[i].begin() + outputOffset,
                 h_vector[i].begin() + outputOffset + mlp->m_Dims.back() * m_Batch / m_VecChannels,
                 h_v + i * mlp->m_Dims.back() * m_Batch / m_VecChannels);
        }
    }
};
#endif

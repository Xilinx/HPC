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
    int outputOffset = 0;
    MLP<t_DataType>* mlp = nullptr;
    MLP<t_DataType>* mlpPad = nullptr;

    const int m_VecChannels = 0;
    const int m_NumChannels = 0;
    const int m_ParEntries = 0;

   public:
    void init() {
        h_weights.resize(m_NumChannels);
        h_vector.resize(m_VecChannels);
    }
    ~MLPKernel() {
        if (mlpPad != nullptr) delete mlpPad;
    }
    MLPKernel(int weightChannels, int vecChannels, int parEntries)
        : m_VecChannels(vecChannels), m_NumChannels(weightChannels), m_ParEntries(parEntries) {
        init();
    }
    MLPKernel(FPGA* fpga, int weightChannels, int vecChannels, int parEntries)
        : Kernel(fpga), m_VecChannels(vecChannels), m_NumChannels(weightChannels), m_ParEntries(parEntries) {
        init();
    }
    void loadModel(MLP<t_DataType>* mlp) {
        this->mlp = mlp;
        int m_NumLayers = mlp->m_NumLayers;
        padMLP();
        h_fcnInstr.resize(m_NumLayers);

        createWeightsBuffer();
        createVecBuffer();
        setModelArgs();

        int weightOffset = 0;
        int biasOffset = 0;

        for (int i = 0; i < mlpPad->m_NumLayers; i++) {
            h_fcnInstr[i].setOutVecSize(mlpPad->m_Dims[i + 1]);
            h_fcnInstr[i].setInVecSize(mlpPad->m_Dims[i]);
            h_fcnInstr[i].setBiasOffset(biasOffset);
            biasOffset += mlpPad->m_Dims[i + 1] * sizeof(t_DataType);
            h_fcnInstr[i].setWeightsOffset(weightOffset);
            weightOffset += mlpPad->m_Dims[i + 1] * mlpPad->m_Dims[i] * sizeof(t_DataType) / m_NumChannels;
            h_fcnInstr[i].setActivation(mlp->m_Layers[i].m_ActFunc);
        }
    }

    double inference(const uint32_t p_batch, t_DataType* h_x, t_DataType* h_v) {
        uint32_t x_size = p_batch * mlp->m_Dims.front();
        setInput(h_x, x_size);
        auto startTime = chrono::high_resolution_clock::now();
        enqueueTask();
        finish();
        auto finishTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();
        getOutput(h_v);
        return t_sec;
    }
    double inference(vector<t_DataType>& h_x, vector<t_DataType>& h_v) {
        uint32_t p_batch = h_x.size() / mlp->m_Dims.front();
        if (h_v.size() < p_batch * mlp->m_Dims.back()) h_v.resize(p_batch * mlp->m_Dims.back());
        inference(p_batch, h_x.data(), h_v.data());
    }

    static double inference(vector<MLPKernel*> kernels, vector<t_DataType>& h_x, vector<t_DataType>& h_v) {
        int l_batchPC = h_x.size() / kernels[0]->mlp->m_Dims.front();
        if (h_v.size() < kernels[0]->mlp->m_Dims.back() * l_batchPC)
            h_v.resize(kernels[0]->mlp->m_Dims.back() * l_batchPC);
        return inference(kernels, l_batchPC, h_x.data(), h_v.data());
    }

    static double inference(vector<MLPKernel*> kernels, uint32_t p_batch, t_DataType* h_x, t_DataType* h_v) {
        uint32_t l_numK = kernels.size();
        const int l_totalSize = p_batch * kernels[0]->mlp->m_Dims.front();
        const int l_kernelSize = (p_batch / l_numK) * kernels[0]->mlp->m_Dims.front();
        vector<int> sizes(l_numK, l_kernelSize);
        sizes.back() = l_totalSize - l_kernelSize * (l_numK - 1);
        for (int i = 0; i < l_numK; i++) {
            kernels[i]->setInput(h_x + i * l_kernelSize, sizes[i]);
        }

        auto startTime = chrono::high_resolution_clock::now();
        for (auto kernel : kernels) kernel->enqueueTask();
        for (auto kernel : kernels) kernel->finish();
        auto finishTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();
        for (int i = 0; i < l_numK; i++) {
            kernels[i]->getOutput(h_v + i * kernels[0]->m_Batch * kernels[0]->mlp->m_Dims.back());
        }

        if (kernels[0]->mlp->m_Layers.back().m_ActFunc == ActFunc_t::SOFTMAX)
            softmax(p_batch, kernels[0]->mlp->m_Dims.back(), h_v);
        return t_sec;
    }

   private:
    void padMLP() {
        mlpPad = new MLP<t_DataType>(mlp->m_NumLayers);
        vector<uint32_t> dims;
        for (int i = 0; i <= mlp->m_NumLayers; i++) {
            int dim = mlp->m_Dims[i];
            dim = (m_ParEntries + dim - 1) / m_ParEntries * m_ParEntries;
            if (i != 0) dim = (m_NumChannels + dim - 1) / m_NumChannels * m_NumChannels;
            dims.push_back(dim);
        }
        mlpPad->setDim(dims);
    }

    void createWeightsBuffer() {
        int biasSize = 0, weightSize = 0;
        for (auto& fcn : mlpPad->m_Layers) {
            biasSize += fcn.m_OutputSize;
            weightSize += fcn.m_OutputSize * fcn.m_InputSize;
        }

        h_bias.resize(biasSize);
        for (int i = 0; i < m_NumChannels; i++) {
            h_weights[i].resize(weightSize / m_NumChannels);
        }

        int biasPos = 0, weightPos = 0;
        for (int i = 0; i < mlpPad->m_NumLayers; i++) {
            FCN<t_DataType>& fcn = mlp->m_Layers[i];
            copy(fcn.m_Bias.begin(), fcn.m_Bias.end(), h_bias.begin() + biasPos);
            biasPos += mlpPad->m_Layers[i].m_OutputSize;

            for (int row = 0; row < fcn.m_OutputSize; row++) {
                copy(fcn.m_Weight.begin() + row * fcn.m_InputSize, fcn.m_Weight.begin() + (row + 1) * fcn.m_InputSize,
                     h_weights[row % m_NumChannels].begin() + weightPos +
                         (row / m_NumChannels) * mlpPad->m_Layers[i].m_InputSize);
            }
            weightPos += mlpPad->m_Layers[i].m_InputSize * mlpPad->m_Layers[i].m_OutputSize / m_NumChannels;
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
        const int l_batchPC = (m_Batch + m_VecChannels - 1) / m_VecChannels;
        outputOffset = (2 * mlpPad->m_MaxVecDim + mlpPad->m_Dims.front()) * l_batchPC;
        int outputSize = mlpPad->m_Dims.back() * l_batchPC;
        int bufferSize = outputOffset + outputSize;
        assert(bufferSize * sizeof(t_DataType) <= 256 * 1024 * 1024);

        for (int i = 0; i < m_Batch; i++) {
            copy(p_x + i * mlp->m_Dims.front(), p_x + (i + 1) * mlp->m_Dims.front(),
                 h_vector[i % m_VecChannels].begin() + (i / m_VecChannels) * mlpPad->m_Dims.front());
        }

        for (int i = 0; i < m_VecChannels; i++) {
            cl_buffer_region buffer_info_x = {0, l_batchPC * mlpPad->m_Dims.front() * sizeof(t_DataType)};
            m_buffer_x.push_back(
                m_buffer_v[i].createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &buffer_info_x));
            cl_buffer_region buffer_info = {outputOffset * sizeof(t_DataType), outputSize * sizeof(t_DataType)};
            m_buffer_y.push_back(
                m_buffer_v[i].createSubBuffer(CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &buffer_info));
        }

        int outputOffset[2] = {mlpPad->m_Dims.front() * l_batchPC * sizeof(t_DataType),
                               (mlpPad->m_MaxVecDim + mlpPad->m_Dims.front()) * l_batchPC * sizeof(t_DataType)};

        h_instr.resize(t_InstrBytes * (1 + mlpPad->m_NumLayers));
        for (int i = 0; i < mlpPad->m_NumLayers; i++) {
            h_fcnInstr[i].setBatch(l_batchPC);
            if (i == 0) {
                h_fcnInstr[i].setInputOffset(0);
            } else
                h_fcnInstr[i].setInputOffset(outputOffset[(i + 1) % 2]);
            if (i == mlpPad->m_NumLayers - 1)
                h_fcnInstr[i].setOutputOffset(this->outputOffset * sizeof(t_DataType));
            else
                h_fcnInstr[i].setOutputOffset(outputOffset[i % 2]);
            h_fcnInstr[i].template store<uint8_t>(h_instr.data() + i * t_InstrBytes);
        }
        setRunArgs();
    }

    void setInput(vector<t_DataType>& p_x) { setInput(p_x.data(), p_x.size()); }

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

    void perf(){
        uint64_t l_last = 0;
        for (int i = 0; i < mlp->m_NumLayers; i++) {
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
    }

    void getOutput(t_DataType* h_v) {
        vector<cl::Memory> h_m;
        h_m.push_back(m_buffer_instr);
        for (auto& y : m_buffer_y) h_m.push_back(y);
        getBuffer(h_m);
        for (int i = 0; i < m_Batch; i++) {
            int vecIndex = i % m_VecChannels;
            int dataOffset = outputOffset + (i / m_VecChannels) * mlpPad->m_Dims.back();
            copy(h_vector[vecIndex].begin() + dataOffset, h_vector[vecIndex].begin() + dataOffset + mlp->m_Dims.back(),
                 h_v + i * mlp->m_Dims.back());
        }
    }
};
#endif

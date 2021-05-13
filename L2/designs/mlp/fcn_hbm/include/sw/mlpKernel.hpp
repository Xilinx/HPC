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
    vector<host_buffer_t<t_DataType> > h_weights;
    vector<host_buffer_t<t_DataType> > h_x;
    host_buffer_t<t_DataType> h_bias;
    host_buffer_t<uint8_t> h_instr;
    vector<FcnInstr<t_InstrBytes> > h_fcnInstr;

    vector<cl::Buffer> m_buffer_W;
    vector<cl::Buffer> m_buffer_x;
    vector<cl::Buffer> m_buffer_y;
    cl::Buffer m_buffer_bias;
    cl::Buffer m_buffer_instr;

    int m_Batch;
    int m_MaxVecDim;
    MLP<t_DataType>* mlp;

    const int m_VecChannels;
    const int m_NumChannels;
    const int m_ParEntries;

   public:
    MLPKernel(FPGA* fpga, int weightChannels, int vecChannels, int parEntries)
        : Kernel(fpga), m_VecChannels(vecChannels), m_NumChannels(weightChannels), m_ParEntries(parEntries) {
        h_weights.resize(m_NumChannels);
        h_x.resize(m_VecChannels);
        h_instr.resize(t_InstrBytes * HPC_maxInstrs);
    }

    void loadModel(MLP<t_DataType>* mlp) {
        this->mlp = mlp;
        int m_NumLayers = mlp->m_NumLayers;
        h_fcnInstr.resize(m_NumLayers);
        m_MaxVecDim = *max_element(mlp->m_Dims.begin() + 1, mlp->m_Dims.end());
        for (int i = 0; i < m_NumLayers; i++) {
            FCN<t_DataType>& fcn = mlp->m_Layers[i];
            h_bias.insert(h_bias.end(), fcn.m_Bias, fcn.m_Bias + fcn.m_OutputSize);
            assert(fcn.m_OutputSize % m_NumChannels == 0);
            assert(fcn.m_InputSize % m_ParEntries == 0);
            for (int row = 0; row < fcn.m_OutputSize; row++)
                h_weights[row % m_NumChannels].insert(h_weights[row % m_NumChannels].end(),
                                                      fcn.m_Weight + row * fcn.m_InputSize,
                                                      fcn.m_Weight + (row + 1) * fcn.m_InputSize);
        }

        for (int i = 0; i < m_NumChannels; i++) {
            m_buffer_W.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_weights[i]));
        }
        m_buffer_bias = createDeviceBuffer(CL_MEM_READ_ONLY, h_bias);

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

    void setInput(host_buffer_t<t_DataType>& p_x) {
        assert(p_x.size() % mlp->m_Dims.front() == 0);
        m_Batch = p_x.size() / mlp->m_Dims.front();
        assert(m_Batch % m_VecChannels == 0);

        int bufferSize = (mlp->m_Dims.front() + 2 * m_MaxVecDim) * m_Batch / m_VecChannels;
        assert(bufferSize * sizeof(t_DataType) <= 256 * 1024 * 1024);

        for (int i = 0; i < m_VecChannels; i++) {
            h_x[i].resize(bufferSize);
            copy(p_x.begin() + i * p_x.size() / m_VecChannels, p_x.begin() + (i + 1) * p_x.size() / m_VecChannels,
                 h_x[i].begin());
            m_buffer_x.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_x[i]));
        }

        int outputOffset[2] = {mlp->m_Dims.front() * m_Batch / m_VecChannels * sizeof(t_DataType),
                               (m_MaxVecDim + mlp->m_Dims.front()) * m_Batch * sizeof(t_DataType) / m_VecChannels};

        for (int i = 0; i < mlp->m_NumLayers; i++) {
            h_fcnInstr[i].setBatch(m_Batch / m_VecChannels);
            if (i == 0) {
                h_fcnInstr[i].setInputOffset(0);
            } else
                h_fcnInstr[i].setInputOffset(outputOffset[(i + 1) % 2]);
            h_fcnInstr[i].setOutputOffset(outputOffset[i % 2]);
            h_fcnInstr[i].template store<uint8_t>(h_instr.data() + i * t_InstrBytes);
        }
    }

    void setArgs() {
        cl_int err;
        vector<cl::Memory> l_buffers;

        m_buffer_instr = createDeviceBuffer(CL_MEM_READ_WRITE, h_instr);
        l_buffers.push_back(m_buffer_instr);
        l_buffers.push_back(m_buffer_bias);
        for (auto x : m_buffer_W) l_buffers.push_back(x);
        for (auto x : m_buffer_x) l_buffers.push_back(x);

        // Setting Kernel Arguments
        uint32_t n_arg = 0;
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_instr));

        for (int i = 0; i < m_NumChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_W[i]));
        }
        for (int i = 0; i < m_VecChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_x[i]));
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_x[i]));
        }
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_bias));

        // Copy input data to device global memory
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(l_buffers, 0)); /* 0 means from host*/
        finish();
    }

    void run(host_buffer_t<t_DataType>& h_v) {
        enqueueTask();
        vector<cl::Memory> h_m;
        h_m.push_back(m_buffer_instr);
        for (auto& r : m_buffer_x) h_m.push_back(r);
        finish();
        cl_int err;
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, CL_MIGRATE_MEM_OBJECT_HOST));
        finish();

        int offset = mlp->m_Dims.front();
        if (mlp->m_NumLayers % 2 == 0) offset += m_MaxVecDim;
        offset *= m_Batch / m_VecChannels;
        for (int i = 0; i < m_VecChannels; i++) {
            h_v.insert(h_v.end(), h_x[i].begin() + offset,
                       h_x[i].begin() + offset + mlp->m_Dims.back() * m_Batch / m_VecChannels);
        }
    }

    double inference(host_buffer_t<t_DataType>& h_x, host_buffer_t<t_DataType>& h_v) {
        auto startTime = chrono::high_resolution_clock::now();
        setInput(h_x);
        setArgs();
        run(h_v);
        auto finishTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finishTime - startTime;
        double t_sec = elapsed.count();

        uint64_t l_last = 0;
        for (int i = 0; i < HPC_maxInstrs; i++) {
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
};
#endif

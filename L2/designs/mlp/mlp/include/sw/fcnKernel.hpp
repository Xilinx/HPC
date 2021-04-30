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

#ifndef XF_HPC_FCN_KERNEL_HPP
#define XF_HPC_FCN_KERNEL_HPP
#include "sw/fpga.hpp"
#include "fcnInstr.hpp"

template <typename, unsigned int, unsigned int>
class FcnKernel;

template <unsigned int t_InstrBytes, unsigned int t_NumChannels, unsigned int t_VecChannels, typename T>
double fcn(FPGA* fpga,
           int p_batch,
           int p_m,
           int p_n,
           host_buffer_t<T>& h_W,
           host_buffer_t<T>& h_x,
           host_buffer_t<T>& h_bias,
           host_buffer_t<T>& h_v) {
    xf::hpc::mlp::FcnInstr<t_InstrBytes> fcnInstr;
    host_buffer_t<uint8_t> h_instr;
    h_instr.resize(t_InstrBytes * HPC_maxInstrs);
    for (int i = 0; i < h_instr.size(); i++) h_instr[i] = 0;

    fcnInstr.setOutVecSize(p_m);
    fcnInstr.setInVecSize(p_n);
    fcnInstr.setBatch(p_batch);
    fcnInstr.setActivation(xf::hpc::mlp::ActFunc_t::SIGMOID);
    fcnInstr.template store<uint8_t>(h_instr.data());
    FcnKernel<T, t_NumChannels, t_VecChannels> fcn(p_batch, p_m, p_n, fpga);
    fcn.getCU("krnl_fcn");
    vector<host_buffer_t<T> > h_vs(HPC_vecChannels);
    fcn.setMem(h_instr, h_W, h_x, h_bias, h_vs);
    double sw_time = fcn.run();
    fcn.getMem();
    for (auto& v : h_vs) h_v.insert(h_v.end(), v.begin(), v.end());

    uint64_t l_last = 0;
    for (int i = 0; i < HPC_maxInstrs; i++) {
        fcnInstr.template load<uint8_t>(h_instr.data() + t_InstrBytes * i);
        uint64_t l_clock = fcnInstr.getClock();
        if (l_clock == 0) break;
        l_clock -= l_last;
        l_last += l_clock;
        if (l_last != 0) {
            cout << "Instruction " << i << ": ";
            // cout << "HW measured cc: " << l_clock << endl;
            cout << "HW efficiency: "
                 << 100.0 * fcnInstr.getOutVecSize() * fcnInstr.getInVecSize() * fcnInstr.getBatch() / HPC_parEntries /
                        t_NumChannels / t_VecChannels / l_clock
                 << "%." << endl;
        }
    }
    cout << "HW measured time " << l_last * HW_CLK << " seconds." << endl;
    return sw_time;
}

template <typename t_DataType, unsigned int t_NumChannels, unsigned int t_VecChannels = 1>
class FcnKernel : public Kernel {
   public:
    FcnKernel(uint32_t p_b, uint32_t p_m, uint32_t p_n, FPGA* fpga) : Kernel(fpga) {
        m_b = p_b;
        m_m = p_m;
        m_n = p_n;
        assert(0 == m_b % t_VecChannels);
        assert(0 == m_m % t_NumChannels);
    }

    void setMem(host_buffer_t<uint8_t>& h_instr,
                host_buffer_t<t_DataType>& h_W,
                host_buffer_t<t_DataType>& h_x,
                host_buffer_t<t_DataType>& h_bias,
                vector<host_buffer_t<t_DataType> >& h_r) {
        assert(h_W.size() == m_m * m_n);
        assert(h_x.size() == m_b * m_n);
        assert(h_bias.size() == m_m);

        cl_int err;
        vector<cl::Memory> l_buffers;
        vector<host_buffer_t<t_DataType> > h_Ws(t_NumChannels);
        setMemA(h_W, h_Ws);

        for (auto x : m_buffer_A) l_buffers.push_back(x);

        vector<host_buffer_t<t_DataType> > h_xs(t_VecChannels);
        setMemX(h_x, h_xs);

        for (auto x : m_buffer_x) l_buffers.push_back(x);

        m_buffer_bias = createDeviceBuffer(CL_MEM_READ_ONLY, h_bias);
        l_buffers.push_back(m_buffer_bias);

        for (auto& r : h_r) {
            r.resize(m_m * m_b / t_VecChannels);
            m_buffer_r.push_back(createDeviceBuffer(CL_MEM_READ_WRITE, r));
        }
        for (auto x : m_buffer_r) l_buffers.push_back(x);

        m_buffer_instr = createDeviceBuffer(CL_MEM_READ_WRITE, h_instr);
        l_buffers.push_back(m_buffer_instr);

        // Setting Kernel Arguments
        uint32_t n_arg = 0;
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_instr));

        for (unsigned int i = 0; i < t_NumChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_A[i]));
        }
        for (unsigned int i = 0; i < t_VecChannels; i++) {
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_x[i]));
            OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_r[i]));
        }
        OCL_CHECK(err, err = m_kernel.setArg(n_arg++, m_buffer_bias));

        // Copy input data to device global memory
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(l_buffers, 0)); /* 0 means from host*/
        finish();
    }

    double run(bool benchmark = false) {
        auto start = chrono::high_resolution_clock::now();
        enqueueTask();
        finish();
        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        double t_sec = elapsed.count();
        return t_sec;
    }

    void getMem() {
        cl_int err;
        vector<cl::Memory> h_m;
        h_m.push_back(m_buffer_instr);
        for (auto& r : m_buffer_r) h_m.push_back(r);
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, CL_MIGRATE_MEM_OBJECT_HOST));
        finish();
    }

   private:
    void setMemX(host_buffer_t<t_DataType>& h_x, vector<host_buffer_t<t_DataType> >& h_xs) {
        if (t_VecChannels == 1) {
            m_buffer_x.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_x));
        } else {
            for (unsigned int i = 0; i < t_VecChannels; i++) {
                h_xs[i].resize(h_x.size() / t_VecChannels);
                copy(h_x.begin() + i * h_x.size() / t_VecChannels, h_x.begin() + (i + 1) * h_x.size() / t_VecChannels,
                     h_xs[i].begin());
                m_buffer_x.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_xs[i]));
            }
        }
    }
    void setMemA(host_buffer_t<t_DataType>& h_W, vector<host_buffer_t<t_DataType> >& h_Ws) {
        if (t_NumChannels == 1) {
            m_buffer_A.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_W));
        } else {
            for (unsigned int i = 0; i < t_NumChannels; i++) h_Ws[i].resize(h_W.size() / t_NumChannels);

            for (unsigned int i = 0; i < m_m; i++)
                copy(h_W.begin() + i * m_n, h_W.begin() + (i + 1) * m_n,
                     h_Ws[i % t_NumChannels].begin() + (i / t_NumChannels) * m_n);
            for (unsigned int i = 0; i < t_NumChannels; i++) {
                m_buffer_A.push_back(createDeviceBuffer(CL_MEM_READ_ONLY, h_Ws[i]));
            }
        }
    }
    uint32_t m_b, m_m, m_n;
    vector<cl::Buffer> m_buffer_A;
    vector<cl::Buffer> m_buffer_x;
    vector<cl::Buffer> m_buffer_r;
    cl::Buffer m_buffer_bias;
    cl::Buffer m_buffer_instr;
};

#endif

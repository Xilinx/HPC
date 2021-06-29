/*
 * Copyright 2019-2021 Xilinx, Inc.
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

#ifndef XFPGA_HPP
#define XFPGA_HPP
#include <iostream>
#include <vector>
#include <regex>
#include <unordered_map>

// This extension file is required for stream APIs
#include "CL/cl_ext_xilinx.h"
// This file is required for OpenCL C++ wrapper APIs
#include "xcl2.hpp"

class FPGA {
   public:
    FPGA();
    FPGA(std::string& deviceName);
    FPGA(unsigned int p_id);
    void init(int p_id, std::string& p_xclbinName);
    void setID(uint32_t id);
    bool xclbin(std::string& binaryFile);
    const cl::Context& getContext() const;
    const cl::CommandQueue& getCommandQueue() const;
    cl::CommandQueue& getCommandQueue();
    const cl::Program& getProgram() const;
    void finish() const;
    cl::Buffer createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size);
    std::vector<cl::Buffer> createDeviceBuffer(cl_mem_flags p_flags,
                                          const std::vector<void*>& p_buffer,
                                          const std::vector<size_t>& p_size);

   protected:
    bool exists(const void* p_ptr) const;
    void getDevices(std::string deviceName);
    FPGA(unsigned int p_id, const std::vector<cl::Device>& devices);

   private:
    unsigned int m_id;
    cl::Device m_device;
    std::vector<cl::Device> m_Devices;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    std::unordered_map<const void*, cl::Buffer> m_bufferMap;
    std::unordered_map<const void*, size_t> m_bufferSzMap;
};

class Kernel {
   public:
    Kernel(FPGA* p_fpga = nullptr);
    void fpga(FPGA* p_fpga);
    void getCU(const std::string& p_name);
    void enqueueTask();
    void finish();
    void getBuffer(std::vector<cl::Memory>& h_m);
    void sendBuffer(std::vector<cl::Memory>& h_m);
    cl::Buffer createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size) const;
    std::vector<cl::Buffer> createDeviceBuffer(cl_mem_flags p_flags, std::vector<void*>& p_buffer, std::vector<size_t>& p_size) const;

   protected:
    FPGA* m_fpga;
    cl::Kernel m_kernel;
    std::vector<cl::Event> m_txEvents, m_runEvents;
};

#endif

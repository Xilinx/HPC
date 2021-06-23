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
#include "xFpga.hpp"
using namespace std;

FPGA::FPGA() {}
FPGA::FPGA(string& deviceName) {
    getDevices(deviceName);
    m_device = m_Devices[m_id];
    m_id = -1;
}
FPGA::FPGA(unsigned int p_id) {
    getDevices("");
    setID(p_id);
}
void FPGA::init(int p_id, string& p_xclbinName) {
    getDevices("");
    setID(p_id);
    xclbin(p_xclbinName);
}

void FPGA::setID(uint32_t id) {
    m_id = id;
    if (m_id >= m_Devices.size()) {
        cout << "Device specified by id = " << m_id << " is not found." << endl;
        throw;
    }
    m_device = m_Devices[m_id];
}

bool FPGA::xclbin(string& binaryFile) {
    cl_int err;
    // Creating Context
    OCL_CHECK(err, m_context = cl::Context(m_device, NULL, NULL, NULL, &err));

    // Creating Command Queue
    OCL_CHECK(err, m_queue = cl::CommandQueue(
                       m_context, m_device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);

    // Creating Program
    OCL_CHECK(err, m_program = cl::Program(m_context, {m_device}, bins, NULL, &err));
    return true;
}
const cl::Context& FPGA::getContext() const {
    return m_context;
}
const cl::CommandQueue& FPGA::getCommandQueue() const {
    return m_queue;
}
cl::CommandQueue& FPGA::getCommandQueue() {
    return m_queue;
}

const cl::Program& FPGA::getProgram() const {
    return m_program;
}

void FPGA::finish() const {
    m_queue.finish();
}

cl::Buffer FPGA::createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size) {
    if (exists(p_buffer)) {
        if (m_bufferSzMap[p_buffer] != p_size) {
            m_bufferMap.erase(p_buffer);
            m_bufferSzMap.erase(p_buffer);
        } else {
            return m_bufferMap[p_buffer];
        }
    }

    size_t l_bufferBytes = p_size;
    cl_int err;
    cl::Buffer l_buffer(m_context, p_flags | CL_MEM_USE_HOST_PTR, l_bufferBytes, p_buffer, &err);
    m_bufferMap[p_buffer] = l_buffer;
    // m_bufferMap.insert(
    //  {p_buffer, cl::Buffer(m_context, p_flags | CL_MEM_USE_HOST_PTR, l_bufferBytes, p_buffer, &err)});
    if (err != CL_SUCCESS) {
        printf("Failed to allocate device buffer!\n");
        throw std::bad_alloc();
    }
    m_bufferSzMap[p_buffer] = p_size;
    return m_bufferMap[p_buffer];
}

vector<cl::Buffer> FPGA::createDeviceBuffer(cl_mem_flags p_flags,
                                            const vector<void*>& p_buffer,
                                            const vector<size_t>& p_size) {
    size_t p_hbm_pc = p_buffer.size();
    vector<cl::Buffer> l_buffer(p_hbm_pc);
    for (unsigned int i = 0; i < p_hbm_pc; i++) {
        l_buffer[i] = createDeviceBuffer(p_flags, p_buffer[i], p_size[i]);
    }
    return l_buffer;
}

bool FPGA::exists(const void* p_ptr) const {
    auto it = m_bufferMap.find(p_ptr);
    return it != m_bufferMap.end();
}
void FPGA::getDevices(string deviceName) {
    cl_int err;
    auto devices = xcl::get_xil_devices();
    auto regexStr = regex(".*" + deviceName + ".*");
    for (auto device : devices) {
        string cl_device_name;
        OCL_CHECK(err, err = device.getInfo(CL_DEVICE_NAME, &cl_device_name));
        if (regex_match(cl_device_name, regexStr)) m_Devices.push_back(device);
    }
    if (0 == m_Devices.size()) {
        cout << "Device specified by name == " << deviceName << " is not found." << endl;
        throw;
    }
}

FPGA::FPGA(unsigned int p_id, const vector<cl::Device>& devices) {
    m_id = p_id;
    m_Devices = devices;
    m_device = m_Devices[m_id];
}

Kernel::Kernel(FPGA* p_fpga) {
    m_fpga = p_fpga;
}

void Kernel::fpga(FPGA* p_fpga) {
    m_fpga = p_fpga;
}

void Kernel::getCU(const string& p_name) {
    cl_int err;
    OCL_CHECK(err, m_kernel = cl::Kernel(m_fpga->getProgram(), p_name.c_str(), &err));
}

void Kernel::enqueueTask() {
    cl_int err;
    cl::Event l_event;
    OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel, &m_txEvents, &l_event));
    m_runEvents.push_back(l_event);
}

void Kernel::finish() {
    m_fpga->finish();
    m_txEvents.clear();
    m_runEvents.clear();
}

void Kernel::getBuffer(vector<cl::Memory>& h_m) {
    cl_int err;
    cl::Event l_event;
    OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, CL_MIGRATE_MEM_OBJECT_HOST,
                                                                            &m_runEvents, &l_event));
}

void Kernel::sendBuffer(vector<cl::Memory>& h_m) {
    cl_int err;
    cl::Event l_event;
    OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, 0, nullptr,
                                                                            &l_event)); /* 0 means from host*/
    m_txEvents.push_back(l_event);
}

cl::Buffer Kernel::createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size) const {
    return m_fpga->createDeviceBuffer(p_flags, p_buffer, p_size);
}

vector<cl::Buffer> Kernel::createDeviceBuffer(cl_mem_flags p_flags,
                                              vector<void*>& p_buffer,
                                              vector<size_t>& p_size) const {
    return m_fpga->createDeviceBuffer(p_flags, p_buffer, p_size);
}


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

#include "xFpga.hpp"

FPGA::FPGA() {}
FPGA::FPGA(std::string& deviceName, bool* p_err) {
    *p_err = getDevices(deviceName);
    m_device = m_Devices[m_id];
    m_id = -1;
}
FPGA::FPGA(bool* p_err) {
    int l_id = getDeviceId();
    *p_err = getDevices("");
    *p_err = *p_err && (setID(l_id));
}
void FPGA::init(std::string& p_xclbinName, bool* p_err) {
    int l_id = getDeviceId();
    *p_err = getDevices("");
    *p_err = *p_err && (setID(l_id));
    *p_err = *p_err && (xclbin(p_xclbinName));
}

int FPGA::getDeviceId() {
    cl_platform_id platform_id = 0;
    cl_platform_id platforms[16] = {0};
    cl_uint platform_count;
    cl_int error;

    clGetPlatformIDs(16, platforms, &platform_count);

    for (cl_uint i = 0; i < platform_count; i++) {
        char platformName[256];
        error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 256, platformName, 0);
        if (error != CL_SUCCESS) {
            exit(EXIT_FAILURE);
        }

        if (strcmp(platformName, "Xilinx") == 0) {
            platform_id = platforms[i];
        }
    }

    cl_uint device_count;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ACCELERATOR, 0, nullptr, &device_count);

    cl_device_id* device_ids = (cl_device_id*)malloc(sizeof(cl_device_id) * device_count);
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ACCELERATOR, device_count, device_ids, nullptr);

    int device_id = 0;
    for (cl_uint i = 0; i < device_count; i++) {
        char device_name[256];
        clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, 256, &device_name, nullptr);
        if (strcmp(device_name, "xilinx_u280_xdma_201920_3") == 0) {
            device_id = i;
        }
    }
    return device_id;
}

bool FPGA::setID(uint32_t id) {
    m_id = id;
    if (m_id >= m_Devices.size()) {
        // std::cout << "Device specified by id = " << m_id << " is not found." << std::endl;
        return false;
    }
    m_device = m_Devices[m_id];
    return true;
}

bool FPGA::xclbin(std::string& binaryFile) {
    cl_int err;
    // Creating Context
    m_context = cl::Context(m_device, NULL, NULL, NULL, &err);

    // Creating Command Queue
    m_queue =
        cl::CommandQueue(m_context, m_device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);

    // Creating Program
    m_program = cl::Program(m_context, {m_device}, bins, NULL, &err);

    if (err != CL_SUCCESS) {
        return false;
    }
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

cl::Buffer FPGA::createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size, bool* p_err) {
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
        // printf("Failed to allocate device buffer!\n");
        // throw std::bad_alloc();
        *p_err = false;
    } else {
        *p_err = true;
    }
    m_bufferSzMap[p_buffer] = p_size;
    return m_bufferMap[p_buffer];
}

std::vector<cl::Buffer> FPGA::createDeviceBuffer(cl_mem_flags p_flags,
                                                 const std::vector<void*>& p_buffer,
                                                 const std::vector<size_t>& p_size,
                                                 bool* p_err) {
    size_t p_hbm_pc = p_buffer.size();
    std::vector<cl::Buffer> l_buffer(p_hbm_pc);
    *p_err = true;
    for (unsigned int i = 0; i < p_hbm_pc; i++) {
        bool l_err = true;
        l_buffer[i] = createDeviceBuffer(p_flags, p_buffer[i], p_size[i], &l_err);
        *p_err = *p_err && l_err;
    }
    return l_buffer;
}

bool FPGA::exists(const void* p_ptr) const {
    auto it = m_bufferMap.find(p_ptr);
    return it != m_bufferMap.end();
}
bool FPGA::getDevices(std::string deviceName) {
    cl_int err = CL_SUCCESS;
    auto devices = xcl::get_xil_devices();
    auto regexStr = std::regex(".*" + deviceName + ".*");
    for (auto device : devices) {
        std::string cl_device_name;
        // OCL_CHECK(err, err = device.getInfo(CL_DEVICE_NAME, &cl_device_name));
        err = device.getInfo(CL_DEVICE_NAME, &cl_device_name);
        if (regex_match(cl_device_name, regexStr)) m_Devices.push_back(device);
    }
    if (0 == m_Devices.size() || err != CL_SUCCESS) {
        // std::cout << "Device specified by name == " << deviceName << " is not found." << std::endl;
        // throw;
        return false;
    } else {
        return true;
    }
}

FPGA::FPGA(const std::vector<cl::Device>& devices) {
    m_id = getDeviceId();
    m_Devices = devices;
    m_device = m_Devices[m_id];
}

Kernel::Kernel(FPGA* p_fpga) {
    m_fpga = p_fpga;
}

void Kernel::fpga(FPGA* p_fpga) {
    m_fpga = p_fpga;
}

bool Kernel::getCU(const std::string& p_name) {
    cl_int err;
    m_kernel = cl::Kernel(m_fpga->getProgram(), p_name.c_str(), &err);
    if (err != CL_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

bool Kernel::enqueueTask() {
    cl_int err;
    cl::Event l_event;
    err = m_fpga->getCommandQueue().enqueueTask(m_kernel, &m_txEvents, &l_event);
    m_runEvents.push_back(l_event);
    if (err != CL_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

void Kernel::finish() {
    m_fpga->finish();
    m_txEvents.clear();
    m_runEvents.clear();
}

bool Kernel::getBuffer(std::vector<cl::Memory>& h_m) {
    cl_int err;
    cl::Event l_event;
    err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, CL_MIGRATE_MEM_OBJECT_HOST, &m_runEvents, &l_event);
    if (err != CL_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

bool Kernel::sendBuffer(std::vector<cl::Memory>& h_m) {
    cl_int err;
    cl::Event l_event;
    err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, 0, nullptr, &l_event); /* 0 means from host*/
    m_txEvents.push_back(l_event);
    if (err != CL_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

cl::Buffer Kernel::createDeviceBuffer(cl_mem_flags p_flags, void* p_buffer, size_t p_size, bool* p_err) const {
    return m_fpga->createDeviceBuffer(p_flags, p_buffer, p_size, p_err);
}

std::vector<cl::Buffer> Kernel::createDeviceBuffer(cl_mem_flags p_flags,
                                                   std::vector<void*>& p_buffer,
                                                   std::vector<size_t>& p_size,
                                                   bool* p_err) const {
    return m_fpga->createDeviceBuffer(p_flags, p_buffer, p_size, p_err);
}

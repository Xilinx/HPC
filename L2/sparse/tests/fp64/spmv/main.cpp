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
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <cassert>

// This file is required for OpenCL C++ wrapper APIs
#include "binFiles.hpp"
#include "utils.hpp"
#include "sw/fp64/spmvHost.hpp"

template <typename T>
struct aligned_allocator {
    using value_type = T;

    aligned_allocator() {}

    aligned_allocator(const aligned_allocator&) {}

    template <typename U>
    aligned_allocator(const aligned_allocator<U>&) {}

    T* allocate(std::size_t num) {
        void* ptr = nullptr;

#if defined(_WINDOWS)
        {
            ptr = _aligned_malloc(num * sizeof(T), 4096);
            if (ptr == nullptr) {
                std::cout << "Failed to allocate memory" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
#else
        {
            if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
        }
#endif
        return reinterpret_cast<T*>(ptr);
    }
    void deallocate(T* p, std::size_t num) {
#if defined(_WINDOWS)
        _aligned_free(p);
#else
        free(p);
#endif
    }
};

template <typename T>
using host_buffer_t = std::vector<T, aligned_allocator<T> >;

int main(int argc, char** argv) {
    if (argc < 5 || argc > 6) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <sigature_path> <vector_path> <mtx_name> [device id]" << std::endl;
        return EXIT_FAILURE;
    }

    int l_idx = 1;

    std::string l_xclbinFile = argv[l_idx++];
    std::string l_sigPath = argv[l_idx++];
    std::string l_vecPath = argv[l_idx++];
    std::string l_mtxName = argv[l_idx++];
    std::string l_sigFilePath = l_sigPath + "/" + l_mtxName;
    std::string l_vecFilePath = l_vecPath + "/" + l_mtxName;

    std::string l_sigFileNames[SPARSE_hbmChannels + 2];
    std::string l_vecFileNames[3];

    for (unsigned int i = 0; i < SPARSE_hbmChannels; ++i) {
        l_sigFileNames[i] = l_sigFilePath + "/nnzVal_" + std::to_string(i) + ".dat";
    }
    l_sigFileNames[SPARSE_hbmChannels] = l_sigFilePath + "/parParam.dat";
    l_sigFileNames[SPARSE_hbmChannels + 1] = l_sigFilePath + "/rbParam.dat";

    l_vecFileNames[0] = l_vecFilePath + "/inVec.dat";
    l_vecFileNames[1] = l_vecFilePath + "/refVec.dat";
    l_vecFileNames[2] = l_vecFilePath + "/outVec.dat";

    // I/O Data Vectors
    host_buffer_t<uint8_t> l_nnzBuf[SPARSE_hbmChannels];
    void* l_nnzBufPtr[SPARSE_hbmChannels];
    size_t l_nnzBufBytes[SPARSE_hbmChannels];
    host_buffer_t<uint8_t> l_parXbuf[2];
    void* l_parXbufPtr[2];
    size_t l_parXbufBytes[2];
    host_buffer_t<uint8_t> l_rbParamBuf;
    size_t l_rbParamBufBytes;
    host_buffer_t<uint8_t> l_yBuf;
    size_t l_yBufBytes;
    host_buffer_t<uint8_t> l_refBuf;

    for (unsigned int i = 0; i < SPARSE_hbmChannels + 2; ++i) {
        size_t l_bytes = getBinBytes(l_sigFileNames[i]);
        if (i < SPARSE_hbmChannels) {
            readBin<uint8_t, aligned_allocator<uint8_t> >(l_sigFileNames[i], l_bytes, l_nnzBuf[i]);
            l_nnzBufPtr[i] = l_nnzBuf[i].data();
            l_nnzBufBytes[i] = l_bytes;
        } else if (i == SPARSE_hbmChannels) {
            readBin<uint8_t, aligned_allocator<uint8_t> >(l_sigFileNames[i], l_bytes,
                                                          l_parXbuf[0]);
            l_parXbufPtr[0] = l_parXbuf[0].data();
            l_parXbufBytes[0] = l_bytes;
        }
        else {
            readBin<uint8_t, aligned_allocator<uint8_t> >(l_sigFileNames[i], l_bytes,
                                                                l_rbParamBuf);
            l_rbParamBufBytes = l_bytes;
        }
    }
    for (unsigned int i = 0; i < 2; ++i) {
        size_t l_bytes = getBinBytes(l_vecFileNames[i]);
        if (i == 0) {
            readBin<uint8_t, aligned_allocator<uint8_t> >(l_vecFileNames[i], l_bytes, l_parXbuf[1]);
            l_parXbufPtr[1] = l_parXbuf[1].data();
            l_parXbufBytes[1] = l_bytes;
        }
        else {
            readBin<uint8_t, aligned_allocator<uint8_t> >(l_vecFileNames[i], l_bytes, l_refBuf);
        }
    }
    unsigned int l_yRows = l_refBuf.size() / sizeof(SPARSE_dataType);
    l_yBufBytes = l_yRows * sizeof(SPARSE_dataType);
    l_yBuf.resize(l_yBufBytes);

    int l_deviceId = 0;
    if (argc > l_idx) l_deviceId = atoi(argv[l_idx++]);

    std::vector<uint32_t> l_info(6);
    readBin<uint32_t>(l_sigFilePath + "/info.dat", 6 * sizeof(uint32_t), l_info.data());
    std::cout << "INFO: Matrix " << l_mtxName << " original m n nnz; padded m n nnz" << std::endl;
    std::cout << "                                ";
    for (unsigned int i = 0; i < 6; ++i) {
        std::cout << l_info[i] << "  ";
    }
    std::cout << std::endl;
    float l_padRatio = (float)(l_info[5]) / (float)(l_info[2]);
    std::cout << "INFO: padding overhead is " << l_padRatio * 100 << "%" << std::endl;

    xilinx_apps::hpc_common::FPGA l_card;
    l_card.setId(l_deviceId);
    l_card.load_xclbin(l_xclbinFile);
    std::cout << "INFO: loading xclbin successfully!" << std::endl;

    xilinx_apps::sparse::SpmvHost<SPARSE_hbmChannels> l_spmvHost;
    l_spmvHost.init(&l_card);
    l_spmvHost.createKernels();

    l_spmvHost.createLoadNnzBufs(l_nnzBufBytes, l_nnzBufPtr);
    l_spmvHost.createLoadParXbufs(l_parXbufBytes, l_parXbufPtr);
    l_spmvHost.createLoadRbParamBufs(l_rbParamBufBytes, l_rbParamBuf.data());
    l_spmvHost.createStoreYbufs(l_yBufBytes, l_yBuf.data());
    l_spmvHost.setStoreYrows(l_yRows);
    l_spmvHost.sendBOs();
    l_spmvHost.run();
    l_spmvHost.getY();
    l_spmvHost.finish();

    writeBin<uint8_t>(l_vecFileNames[2], l_yRows * sizeof(SPARSE_dataType),
                      reinterpret_cast<uint8_t*>(&(l_yBuf[0])));
    int l_errs = 0;
    compare<SPARSE_dataType>(l_yRows, reinterpret_cast<SPARSE_dataType*>(l_yBuf.data()),
                             reinterpret_cast<SPARSE_dataType*>(l_refBuf.data()), l_errs, true);
    if (l_errs == 0) {
        std::cout << "INFO: Test pass!" << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cout << "ERROR: Test failed! Out of total " << l_yRows << " entries, there are " << l_errs << " mismatches."
             << std::endl;
        return EXIT_FAILURE;
    }
}

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
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cassert>
#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// This file is required for OpenCL C++ wrapper APIs
#include "impl/xans_mem.hpp"
#include "sw/utils.hpp"
#include "sw/binFiles.hpp"
#include "dSpmvComputeHost.hpp"
#include "dSpmvStoreYhost.hpp"

int main(int argc, char** argv) {
    constexpr unsigned int t_MemBytes = SPARSE_hbmMemBits / 8;
    constexpr unsigned int t_NetBytes = XANS_netDataBits/8;
    constexpr unsigned int t_NetDataNum = t_NetBytes / sizeof(SPARSE_dataType);

    if (argc < 4 || (std::string(argv[1]) == "-help")) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " <socket_file> <ip_file> [signiture_path] <vector_path> <mtx_name>" << std::endl;
        std::cout << "dspmv.exe -help";
        std::cout << "    -- print out this usage" << std::endl;
        return EXIT_FAILURE;
    }

    int l_idx = 1;
    std::string l_sockFileName = argv[l_idx++];
    std::string l_ipFileName = argv[l_idx++];

    std::string l_hostName;
    std::string l_xclbinName;
    int l_devId = 0;
    std::string l_myIPstr;
    uint32_t l_myIP = 0;
    uint32_t l_myID = 0;
    std::vector<uint32_t> l_ipTable;
    std::vector<uint32_t> l_socketTable;

    std::ifstream l_sockFile(l_sockFileName);
    std::string l_myHostName;
    std::string l_devIdStr;
    if (l_sockFile.is_open()) {
        std::string l_lineStr;
        if (getline(l_sockFile, l_lineStr)) {
            std::istringstream l_strStream(l_lineStr);
            l_strStream >> l_hostName >> l_myIPstr >> l_xclbinName >> l_devIdStr;
            l_myIP = xilinx_apps::xans::getIPaddr(l_myIPstr);
            l_devId = std::stoi(l_devIdStr);
            std::cout << "INFO: My ip address is: " << l_myIPstr << std::endl;
        }
        while (getline(l_sockFile, l_lineStr)) {
            std::istringstream l_strStream(l_lineStr);
            std::string l_sockIDstr, l_srcIPstr, l_desIPstr;
            l_strStream >> l_sockIDstr >> l_srcIPstr >> l_desIPstr;
            std::cout << "Sockets INFO: " << l_sockIDstr << " " << l_srcIPstr << " " << l_desIPstr << std::endl;
            l_socketTable.push_back(xilinx_apps::xans::getIPaddr(l_desIPstr));
        }
        l_sockFile.close();
    }

    std::ifstream l_ipFile(l_ipFileName);
    if (l_ipFile.is_open()) {
        std::string l_ipStr;
        while (getline(l_ipFile, l_ipStr)) {
            uint32_t l_ipAddr = xilinx_apps::xans::getIPaddr(l_ipStr);
            if (l_ipAddr == l_myIP) {
                l_myID = l_ipTable.size();
            }
            l_ipTable.push_back(l_ipAddr);
        }
        l_ipFile.close();
    }

    uint32_t l_maxID = l_ipTable.size() - 1;
    int l_infID = 0;

    xilinx_apps::hpc_common::FPGA l_card;
    xilinx_apps::xans::XansImp<XANS_numInf, t_MemBytes> l_xansHost;
    l_card.setId(l_devId);
    l_card.load_xclbin(l_xclbinName);
    std::cout << "INFO: loading xclbin successfully!" << std::endl;
    l_xansHost.init(&l_card);

    l_xansHost.updateXnikIpAddress(l_infID, l_myIP);
    l_xansHost.arpDiscovery(l_infID);
    std::map<int, std::pair<std::string, std::string> > table = l_xansHost.readARPTable(l_infID);
    std::cout << "INFO: ARP table for device " << l_devId << " is " << std::endl;
    for (auto& item : table) {
        std::cout << item.first << ": {MAC address: " << item.second.first << ", IP address: " << item.second.second
                  << "}" << std::endl;
    }
    l_xansHost.setXnikSockets(l_myIP, l_socketTable);
    if (l_myID == 0) { // compute 
        std::string l_sigPath = argv[l_idx++];
        std::string l_vecPath = argv[l_idx++];
        std::string l_mtxName = argv[l_idx++];
        std::string l_datFileName[SPARSE_hbmChannels];
        std::string l_parParamFileName;
        std::string l_rbParamFileName;
        std::string l_xFileName;
        for (unsigned int i=0; i<SPARSE_hbmChannels; ++i) {
            l_datFileName[i] = l_sigPath + "/" + l_mtxName + "/nnzVal_" + std::to_string(i) + ".dat";
        }
        l_parParamFileName = l_sigPath + "/" + l_mtxName + "/parParam.dat";
        l_rbParamFileName = l_sigPath + "/" + l_mtxName + "/rbParam.dat";
        l_xFileName = l_vecPath + "/" + l_mtxName + "/inVec.dat";

        std::vector<uint8_t, alignedAllocator<uint8_t> > l_nnzBuf[SPARSE_hbmChannels];
        void* l_nnzBufPtr[SPARSE_hbmChannels];
        size_t l_nnzBufBytes[SPARSE_hbmChannels];

        std::vector<uint8_t, alignedAllocator<uint8_t> > l_parXbuf[2]; 
        void* l_parXbufPtr[2];
        size_t l_parXbufBytes[2];
        
        std::vector<uint8_t, alignedAllocator<uint8_t> > l_rbParamBuf;
        size_t l_rbParamBufBytes;

        for (unsigned int i = 0; i < SPARSE_hbmChannels; ++i) {
            l_nnzBufBytes[i]  = getBinBytes(l_datFileName[i]);
            l_nnzBuf[i].resize(l_nnzBufBytes[i]);
            readBin<uint8_t>(l_datFileName[i], l_nnzBufBytes[i], l_nnzBuf[i].data());
            l_nnzBufPtr[i] = l_nnzBuf[i].data();
        }
        l_parXbufBytes[0] = getBinBytes(l_parParamFileName);
        l_parXbuf[0].resize(l_parXbufBytes[0]);
        readBin<uint8_t>(l_parParamFileName, l_parXbufBytes[0], l_parXbuf[0].data());
        l_parXbufPtr[0] = l_parXbuf[0].data();

        l_parXbufBytes[1] = getBinBytes(l_xFileName);
        l_parXbuf[1].resize(l_parXbufBytes[1]);
        readBin<uint8_t>(l_xFileName, l_parXbufBytes[1], l_parXbuf[1].data());
        l_parXbufPtr[1] = l_parXbuf[1].data();

        l_rbParamBufBytes = getBinBytes(l_rbParamFileName);
        l_rbParamBuf.resize(l_rbParamBufBytes);
        readBin<uint8_t>(l_rbParamFileName, l_rbParamBufBytes, l_rbParamBuf.data());
        
        std::vector<uint32_t> l_info(6);
        readBin<uint32_t>(l_sigPath + "/" + l_mtxName + "/info.dat", 6 * sizeof(uint32_t), l_info.data());
        std::cout << "INFO: Matrix " << l_mtxName << " original m n nnz; padded m n nnz" << std::endl;
        std::cout << "                                ";
        for (unsigned int i = 0; i < 6; ++i) {
            std::cout << l_info[i] << "  ";
        }
        std::cout << std::endl;
        float l_padRatio = (float)(l_info[5]) / (float)(l_info[2]);
        std::cout << "INFO: padding overhead is " << l_padRatio * 100 << "%" << std::endl;

        unsigned int l_rows = l_info[0];
        unsigned int l_paddedRows = ((l_rows + SPARSE_parEntries - 1) / SPARSE_parEntries) * SPARSE_parEntries;
        unsigned int l_netRowBks = (l_paddedRows + t_NetDataNum -1) / t_NetDataNum;
        unsigned int l_msgBytes = l_netRowBks * t_NetBytes;
        l_msgBytes = l_msgBytes + t_NetBytes;
 
        xilinx_apps::dspmv::dSpmvComputeHost<SPARSE_hbmChannels> l_computeHost;
        l_computeHost.init(&l_card);
        l_computeHost.createKernels();
        l_computeHost.createLoadNnzBufs(l_nnzBufBytes, l_nnzBufPtr);
        l_computeHost.createLoadParXbufs(l_parXbufBytes, l_parXbufPtr);
        l_computeHost.createLoadRbParamBufs(l_rbParamBufBytes, l_rbParamBuf.data());
        l_computeHost.setKrnTransYargs(0, l_paddedRows);
        l_computeHost.sendBOs();
        l_computeHost.run();
        std::cout << "INFO: launch dspmv compute kernel" << std::endl;

        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID + 1], xilinx_apps::xans::MPI_OPCODE::MPI_SEND, l_msgBytes);
        l_xansHost.addInstCtl(l_myIP, xilinx_apps::xans::MPI_OPCODE::MPI_FIN);
        l_xansHost.setXnikMem(l_myIP);
        l_xansHost.startXnik(l_myIP);
        std::cout <<"INFO: launch XNIK" << std::endl;
        std::cout <<"INFO: send " << l_msgBytes << " bytes." << std::endl;

        l_computeHost.finish();
        std::cout << "INFO: finish dspmv compute." << std::endl;
        l_xansHost.finish();
        std::cout << "INFO: finish XNIK." << std::endl;
    } else { //store
        std::string l_vecPath = argv[l_idx++];
        std::string l_mtxName = argv[l_idx++];
        std::string l_refVecFileName = l_vecPath + "/" + l_mtxName + "/refVec.dat";
        std::string l_outVecFileName = l_vecPath + "/" + l_mtxName  +"/outVec.dat";
       
        std::vector<uint8_t, alignedAllocator<uint8_t> > l_yBuf;
        size_t l_yBufBytes;
        std::vector<uint8_t, alignedAllocator<uint8_t> > l_refBuf;
        size_t l_refBufBytes;

        l_refBufBytes = getBinBytes(l_refVecFileName);
        l_refBuf.resize(l_refBufBytes);
        readBin<uint8_t>(l_refVecFileName, l_refBufBytes, l_refBuf.data());

        unsigned int l_yRows = l_refBufBytes / sizeof(SPARSE_dataType);
        unsigned int l_yRowBks = (l_yRows + t_NetDataNum -1) / t_NetDataNum;
        unsigned int l_yNetAlignedRows = l_yRowBks * t_NetDataNum;
        l_yBufBytes = l_yNetAlignedRows * sizeof(SPARSE_dataType);
        l_yBuf.resize(l_yBufBytes);
        std::cout << "INFO: load data successfully!" << std::endl;

        unsigned int l_msgBytes = l_yBufBytes + t_NetBytes;
        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID - 1], xilinx_apps::xans::MPI_OPCODE::MPI_RECEIVE, l_msgBytes);
        l_xansHost.addInstCtl(l_myIP, xilinx_apps::xans::MPI_OPCODE::MPI_FIN);
        l_xansHost.setXnikMem(l_myIP);
        l_xansHost.startXnik(l_myIP);
        std::cout << "INFO: launch XNIK kernel." << std::endl;
        std::cout << "INFO: waiting to receive " << l_msgBytes <<  " bytes." << std::endl;

        xilinx_apps::dspmv::dSpmvStoreYhost<SPARSE_hbmChannels> l_storeYhost;
        l_storeYhost.init(&l_card);
        l_storeYhost.createKernels();
        l_storeYhost.createStoreYbufs(l_yBufBytes, l_yBuf.data());
        l_storeYhost.setStoreYargs(SPARSE_compNodes);
        l_storeYhost.run();
        std::cout << "INFO: launch storeY kernel." << std::endl;
  
        
        l_storeYhost.getY();
        std::cout << "INFO: finish dspmv store." << std::endl;

        l_xansHost.finish();
        std::cout << "INFO: finish XNIK." << std::endl;

        writeBin<uint8_t>(l_outVecFileName, l_yRows * sizeof(SPARSE_dataType),
                      reinterpret_cast<uint8_t*>(l_yBuf.data()));

        int l_errs = 0;
        compare<SPARSE_dataType>(l_yRows, reinterpret_cast<SPARSE_dataType*>(l_yBuf.data()),
                             reinterpret_cast<SPARSE_dataType*>(l_refBuf.data()), l_errs, true);
        if (l_errs != 0) {
            std::cout << "ERROR: Test failed! Out of total " << l_yRows << " entries, there are " << l_errs << " mismatches." << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "Test Pass!" << std::endl;
    }
    return EXIT_SUCCESS;
}

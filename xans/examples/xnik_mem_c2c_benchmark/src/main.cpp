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
#include "basicHost.hpp"
#include "echoHost.hpp"

constexpr unsigned int t_MemBytes = XANS_memBits / 8;
int main(int argc, char** argv) {
    if (argc != 5 || (std::string(argv[1]) == "-help")) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " <socket_file> <ip_file> <bytes> <Clock Period>" << std::endl;
        std::cout << "alveo_relay.exe -help";
        std::cout << "    -- print out this usage" << std::endl;
        return EXIT_FAILURE;
    }

    int l_idx = 1;
    std::string l_sockFileName = argv[l_idx++];
    std::string l_ipFileName = argv[l_idx++];
    unsigned int l_msgBytes = atoi(argv[l_idx++]);
    double l_clockPeriod = atof(argv[l_idx++]);

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
    std::vector<uint8_t> l_dat(l_msgBytes);
    srand(time(0));
    for (unsigned int i = 0; i < l_msgBytes; ++i) {
        l_dat[i] = i; // rand();
    }
    if (l_myID == 0) { // driver
        basicHost<XANS_netDataBits> l_basicHost;
        l_basicHost.init(&l_card);
        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID + 1], xilinx_apps::xans::MPI_OPCODE::MPI_SEND, l_msgBytes);
        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID + 1], xilinx_apps::xans::MPI_OPCODE::MPI_RECEIVE, l_msgBytes);
        l_xansHost.addInstCtl(l_myIP, xilinx_apps::xans::MPI_OPCODE::MPI_FIN);
        l_xansHost.setXnikMem(l_myIP);
        l_xansHost.startXnik(l_myIP);

        l_basicHost.createKrnS2mm();
        std::cout << "create kernel s2mm done." << std::endl;
        l_basicHost.createS2mmBufs();
        std::cout << "create s2mm buffer done." << std::endl;
        l_basicHost.runKrnS2mm(l_msgBytes);
        std::cout << "kick off s2mm Kernel." << std::endl;

        l_basicHost.createKrnCount();
        std::cout << "create kernel counter done." << std::endl;
        l_basicHost.createCountBufs(32);
        std::cout << "create counter buffer done." << std::endl;
        l_basicHost.runKrnCount();
        std::cout << "kick off counter Kernel." << std::endl;

        l_basicHost.createKrnmm2S();
        std::cout << "create kernel mm2s done." << std::endl;
        l_basicHost.runKrnmm2S(l_msgBytes);
        std::cout << "krnl_mm2s sent out " << l_msgBytes << " bytes." << std::endl;

        void* l_counterRes = l_basicHost.getCountRes();
        void* l_s2mmRes = l_basicHost.getS2mmRes();
        uint32_t* l_errRes = (uint32_t*)l_s2mmRes;
        uint32_t l_errs = l_errRes[0];
        if (l_errs != 0) {
            std::cout << "ERROR: receive results doesn't match! In total " << l_errs << " mismatches." << std::endl;
            for (unsigned int i = 1; i <= l_errs; ++i) {
                l_errRes += 16;
                std::cout << "ERROR: index/ref = " << l_errRes[1] << " output = " << l_errRes[0] << std::endl;
            }
        } else {
            std::cout << "Test Pass!" << std::endl;
        }

        double l_roundTrip_latency =
            (double)((reinterpret_cast<uint64_t*>(l_counterRes))[0] * l_clockPeriod / (double)1000 -
                    l_msgBytes / (double)12500);
        double l_totalTime = (double)((reinterpret_cast<uint64_t*>(l_counterRes))[3] * l_clockPeriod / (double)1000);
        double l_throughput = (double)(l_msgBytes * 2 / (double)l_totalTime / (double)1000);
        std::cout << "INFO: latency = " << (reinterpret_cast<uint64_t*>(l_counterRes))[0] << " cycles" << std::endl;
        std::cout << "INFO: sending time = " << (reinterpret_cast<uint64_t*>(l_counterRes))[1] << " cycles"
                  << std::endl;
        std::cout << "INFO: receiving time = " << (reinterpret_cast<uint64_t*>(l_counterRes))[2] << " cycles"
                  << std::endl;
        std::cout << "INFO: totoal time = " << (reinterpret_cast<uint64_t*>(l_counterRes))[3] << " cycles" << std::endl;
        std::cout << "DATA_CSV:, Data size [bytes], Latency [Cycles], Total Time [Cycles], Round Trip Latency [us], "
                     "Total Time [us], Point to Point Latency [us], Throughput [GB/Sec]"
                  << std::endl;
        std::cout << "DATA_CSV:, " << l_msgBytes << ", " << (reinterpret_cast<uint64_t*>(l_counterRes))[0] << ", "
                  << (reinterpret_cast<uint64_t*>(l_counterRes))[3] << ", " << l_roundTrip_latency << ", "
                  << l_totalTime << ", " << l_roundTrip_latency / 2 << ", " << l_throughput << std::endl;
        l_xansHost.finish();
    } else {
        echoHost<XANS_netDataBits> l_echoHost;
        l_echoHost.init(&l_card);
        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID - 1], xilinx_apps::xans::MPI_OPCODE::MPI_RECEIVE, l_msgBytes);
        l_xansHost.addInstMPI(l_myIP, l_ipTable[l_myID - 1], xilinx_apps::xans::MPI_OPCODE::MPI_SEND, l_msgBytes);
        l_xansHost.addInstCtl(l_myIP, xilinx_apps::xans::MPI_OPCODE::MPI_FIN);
        l_xansHost.setXnikMem(l_myIP);
        l_xansHost.startXnik(l_myIP);

        l_echoHost.createKrnEcho();
        std::cout << "create kernel echo done." << std::endl;
        l_echoHost.createEchoBufs();
        std::cout << "create echo buffer done." << std::endl;
        l_echoHost.runKrnEcho(l_msgBytes);
        std::cout << "kick off echo Kernel." << std::endl;

        void* l_echoRes = l_echoHost.getEchoRes();
        uint32_t l_errs = ((uint32_t*)(l_echoRes))[0];
        if (l_errs != 0) {
            std::cout << "ERROR: receive results in krnl_echo doesn't match!" << std::endl;
            l_xansHost.finish();
            return EXIT_FAILURE;
        }
        std::cout << "Test Pass!" << std::endl;
        l_xansHost.finish();
    }
    return EXIT_SUCCESS;
}

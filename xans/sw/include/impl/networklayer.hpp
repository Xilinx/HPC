
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

#ifndef NETWORKLAYER_HPP
#define NETWORKLAYER_HPP

#include <vector>
#include "sw/xNativeFPGA.hpp"

namespace xilinx_apps {
namespace xans {

constexpr size_t mac_address = 0x0010;
constexpr size_t ip_address = 0x0018;
constexpr size_t gateway = 0x001C;
constexpr size_t arp_discovery = 0x3010;
constexpr size_t arp_mac_addr_offset = 0x3800;
constexpr size_t arp_ip_addr_offset = 0x3400;
constexpr size_t arp_valid_offset = 0x3100;
constexpr size_t udp_theirIP_offset = 0x2010;
constexpr size_t udp_theirPort_offset = 0x2090;
constexpr size_t udp_myPort_offset = 0x2110;
constexpr size_t udp_valid_offset = 0x2190;
constexpr size_t udp_number_sockets = 0x2210;
constexpr size_t udp_in_packets = 0x10D0;
constexpr size_t udp_out_packets = 0x1100;
constexpr size_t udp_app_in_packets = 0x10E8;
constexpr size_t udp_app_out_packets = 0x1118;

constexpr size_t udp_in_bytes = 0x10C8;
constexpr size_t udp_out_bytes = 0x10F8;
constexpr size_t udp_app_in_bytes = 0x10E0;
constexpr size_t udp_app_out_bytes = 0x1110;

constexpr size_t ethhi_out_bytes = 0x1098;
constexpr size_t eth_out_bytes = 0x10b0;
struct SocketType {
    std::string theirIP;
    uint32_t theirIPint;
    //uint16_t theirPort = 38746; //for cpu
    uint16_t theirPort = 62781; 
    uint16_t myPort = 62781;
    bool valid = false;
};

uint32_t getIPaddr(const std::string p_ipAddrStr);
std::string getIPstr(const uint32_t p_ipAddr);

class KernelNetworklayer : public xilinx_apps::hpc_common::IP {
   public:
    KernelNetworklayer() = default;
    void initCU(const unsigned int p_id);
    uint32_t updateIPAddress(const std::string p_ipAddrStr);
    void updateIPAddress(const uint32_t p_ipAddrStr);
    void arpDiscovery();
    std::map<int, std::pair<std::string, std::string> > readARPTable(int num_entries);
    void configureSocket(int index, std::string p_theirIP, uint16_t p_theirPort, uint16_t p_myPort, bool p_valid);
    SocketType getHostSocket(int index);
    std::map<int, SocketType> populateSocketTable();
    void populateSocketTable(std::vector<SocketType>& p_socketTable);
    void printSocketTable(const unsigned int p_numSockets);
    uint32_t getUDPinPkts();
    uint32_t getUDPoutPkts();
    uint32_t getUDPappInPkts();
    uint32_t getUDPappOutPkts();

   private:
    SocketType sockets[16];
};
}
}

#endif

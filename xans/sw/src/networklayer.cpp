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

#include "impl/networklayer.hpp"
#include "impl/xansException.hpp"
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>

namespace xilinx_apps {
namespace xans {

uint32_t getIPaddr(const std::string p_ipAddrStr) {
    std::vector<std::string> l_ipAddrVec;
    std::stringstream l_str(p_ipAddrStr);
    std::string l_ipAddrStr;
    if (std::getline(l_str, l_ipAddrStr, '.').fail()) {
        throw xansInvalidIp("IP address is ill-formed.");
        return 0;
    } else {
        l_ipAddrVec.push_back(l_ipAddrStr);
    }
    while (std::getline(l_str, l_ipAddrStr, '.')) {
        l_ipAddrVec.push_back(l_ipAddrStr);
    }
    if (l_ipAddrVec.size() != 4) {
        throw xansInvalidIp("IP address is ill-formed.");
        return 0;
    }
    uint32_t l_ipAddr = 0;
    for (auto i = 0; i < 4; ++i) {
        l_ipAddr = l_ipAddr << 8;
        uint32_t l_val = std::stoi(l_ipAddrVec[i]);
        if (l_val > 255) {
            std::string l_errStr = l_ipAddrVec[i] + " should be less than 255.";
            throw xansInvalidIp(l_errStr);
            return 0;
        }
        l_ipAddr += l_val;
    }
    return l_ipAddr;
}

std::string getIPstr(const uint32_t p_ipAddr) {
    std::string l_ipStr;
    for (auto i=0; i<4; ++i) {
        uint32_t l_ipAddr = p_ipAddr;
        l_ipAddr = l_ipAddr >> (4-i-1)*8;
        uint8_t l_digit = l_ipAddr & 0xff;
        l_ipStr = l_ipStr + std::to_string(l_digit);
        if (i != 3) {
            l_ipStr += ".";
        }
    }
    return l_ipStr;
}

void KernelNetworklayer::initCU(const unsigned int p_id) {
    std::string l_cuName = "networklayer:{networklayer_" + std::to_string(p_id) + "}";
    this->getIP(l_cuName);
}


uint32_t KernelNetworklayer::updateIPAddress(const std::string p_ipAddrStr) {
    uint32_t p_ipAddr = getIPaddr(p_ipAddrStr);
    this->writeReg(ip_address, p_ipAddr);
    uint32_t l_gatewayAddr = (p_ipAddr & 0xFFFFFF00) + 1;
    this->writeReg(gateway, l_gatewayAddr);
    uint32_t l_curMacAddr;
    l_curMacAddr = this->readReg(mac_address);
    uint32_t l_newMacAddr = (l_curMacAddr & 0xFFFFFFFFFF00) + (p_ipAddr & 0xFF);
    this->writeReg(mac_address, l_newMacAddr);
    return p_ipAddr;
}

void KernelNetworklayer::updateIPAddress(const uint32_t p_ipAddr) {
    this->writeReg(ip_address, p_ipAddr);
    uint32_t l_gatewayAddr = (p_ipAddr & 0xFFFFFF00) + 1;
    this->writeReg(gateway, l_gatewayAddr);
    uint32_t l_curMacAddr;
    l_curMacAddr = this->readReg(mac_address);
    uint32_t l_newMacAddr = (l_curMacAddr & 0xFFFFFFFFFF00) + (p_ipAddr & 0xFF);
    this->writeReg(mac_address, l_newMacAddr);
}

void KernelNetworklayer::arpDiscovery() {
    this->writeReg(arp_discovery, 0);
    this->writeReg(arp_discovery, 1);
    this->writeReg(arp_discovery, 0);
}

unsigned long long _byteOrderingEndianess(unsigned long long num, int length = 4) {
    unsigned long long aux = 0;
    for (int i = 0; i < length; i++) {
        unsigned long long byte_index = num >> ((length - 1 - i) * 8) & 0xFF;
        aux += byte_index << (i * 8);
    }
    return aux;
}

/*
 * Read the ARP table from the FPGA return a map
 */
std::map<int, std::pair<std::string, std::string> > KernelNetworklayer::readARPTable(int num_entries) {
    uint32_t mac_addr_offset = arp_mac_addr_offset;
    uint32_t ip_addr_offset = arp_ip_addr_offset;
    uint32_t valid_addr_offset = arp_valid_offset;
    std::map<int, std::pair<std::string, std::string> > table;
    unsigned long long valid_entry;

    for (int i = 0; i < num_entries; i++) {
        if ((i % 4) == 0) {
            valid_entry = this->readReg(valid_addr_offset + (i / 4) * 4);
        }
        unsigned long long isvalid = (valid_entry >> ((i % 4) * 8)) & 0x1;
        if (isvalid) {
            unsigned long long mac_lsb = this->readReg(mac_addr_offset + (i * 2 * 4));
            unsigned long long mac_msb = this->readReg(mac_addr_offset + ((i * 2 + 1) * 4));
            unsigned long long ip_addr = this->readReg(ip_addr_offset + (i * 4));
            unsigned long long mac_addr = pow(2, 32) * mac_msb + mac_lsb;
            unsigned long long mac_hex = _byteOrderingEndianess(mac_addr, 6);
            std::stringstream mac_hex_stringstream;
            mac_hex_stringstream << std::hex << mac_hex << std::dec;
            std::string mac_hex_string = mac_hex_stringstream.str();
            mac_hex_string = std::string(12 - mac_hex_string.length(), '0') + mac_hex_string;
            std::string mac_str = "";
            for (int j = 0; j < (int)mac_hex_string.length(); j++) {
                mac_str = mac_str + mac_hex_string.at(j);
                if ((j % 2 != 0) && (j != (int)mac_hex_string.length() - 1)) {
                    mac_str = mac_str + ":";
                }
            }
            unsigned long long ip_addr_print = _byteOrderingEndianess(ip_addr);
            unsigned char ipBytes[4];
            ipBytes[0] = ip_addr_print & 0xFF;
            ipBytes[1] = (ip_addr_print >> 8) & 0xFF;
            ipBytes[2] = (ip_addr_print >> 16) & 0xFF;
            ipBytes[3] = (ip_addr_print >> 24) & 0xFF;
            std::stringstream ip_addr_printstream;
            ip_addr_printstream << int(ipBytes[3]) << "." << int(ipBytes[2]) << "." << int(ipBytes[1]) << "."
                                << int(ipBytes[0]);
            std::string ip_addr_print_string = ip_addr_printstream.str();
            table.insert({i, {mac_str, ip_addr_print_string}});
        }
    }
    return table;
}

void KernelNetworklayer::configureSocket(
    int index, std::string p_theirIP, uint16_t p_theirPort, uint16_t p_myPort, bool p_valid) {
    xilinx_apps::xans::SocketType l_socket = {p_theirIP, getIPaddr(p_theirIP), p_theirPort, p_myPort, p_valid};
    this->sockets[index] = l_socket;
}

xilinx_apps::xans::SocketType KernelNetworklayer::getHostSocket(int index) {
    return this->sockets[index];
}

std::map<int, SocketType> KernelNetworklayer::populateSocketTable() {
    uint32_t theirIP_offset = udp_theirIP_offset;
    uint16_t theirPort_offset = udp_theirPort_offset;
    uint16_t myPort_offset = udp_myPort_offset;
    uint16_t valid_offset = udp_valid_offset;

    int numSocketsHW = this->readReg(udp_number_sockets);

    if (numSocketsHW < 16) {
        std::string errMsg = "Socket list length " + std::to_string(16) +
                             " is bigger than the number of sockets in hardware " + std::to_string(numSocketsHW);
        throw xansInvalidValue(errMsg);
    }

    for (int i = 0; i < numSocketsHW; i++) {
        uint32_t ti_offset = theirIP_offset + i * 8;
        uint32_t tp_offset = theirPort_offset + i * 8;
        uint32_t mp_offset = myPort_offset + i * 8;
        uint32_t v_offset = valid_offset + i * 8;

        uint32_t theirIP = 0;
        if (!this->sockets[i].theirIP.empty()) {
            theirIP = this->sockets[i].theirIPint;
        }
        this->writeReg(ti_offset, theirIP);
        this->writeReg(tp_offset, this->sockets[i].theirPort);
        this->writeReg(mp_offset, this->sockets[i].myPort);
        this->writeReg(v_offset, this->sockets[i].valid);
    }

    std::map<int, xilinx_apps::xans::SocketType> socket_dict;

    for (int i = 0; i < numSocketsHW; i++) {
        uint32_t ti_offset = theirIP_offset + i * 8;
        uint32_t tp_offset = theirPort_offset + i * 8;
        uint32_t mp_offset = myPort_offset + i * 8;
        uint32_t v_offset = valid_offset + i * 8;
        uint32_t isvalid = this->readReg(v_offset);
        if (isvalid) {
            uint32_t ti = this->readReg(ti_offset);
            uint32_t tp = this->readReg(tp_offset);
            uint32_t mp = this->readReg(mp_offset);

            unsigned char ipBytes[4];
            ipBytes[0] = ti & 0xFF;
            ipBytes[1] = (ti >> 8) & 0xFF;
            ipBytes[2] = (ti >> 16) & 0xFF;
            ipBytes[3] = (ti >> 24) & 0xFF;
            std::stringstream ti_printstream;
            ti_printstream << int(ipBytes[3]) << "." << int(ipBytes[2]) << "." << int(ipBytes[1]) << "."
                           << int(ipBytes[0]);
            xilinx_apps::xans::SocketType l_socket = {ti_printstream.str(), ti, tp, mp, true};
            socket_dict.insert({i, l_socket});
        }
    }

    return socket_dict;
}

void KernelNetworklayer::printSocketTable(const unsigned int p_numSockets) {
    uint32_t theirIP_offset = udp_theirIP_offset;
    uint16_t theirPort_offset = udp_theirPort_offset;
    uint16_t myPort_offset = udp_myPort_offset;
    uint16_t valid_offset = udp_valid_offset;
    for (unsigned int i=0; i<p_numSockets; ++i) {
        uint32_t ti_offset = theirIP_offset + i * 8;
        uint32_t tp_offset = theirPort_offset + i * 8;
        uint32_t mp_offset = myPort_offset + i * 8;
        uint32_t v_offset = valid_offset + i * 8;
        uint32_t isValid = this->readReg(v_offset);
        if (isValid == 0) {
            throw xansInvalidValue("Socket not set properly.");
        }
        else {
            uint32_t ti = this->readReg(ti_offset);
            uint32_t tp = this->readReg(tp_offset);
            uint32_t mp = this->readReg(mp_offset);

            unsigned char ipBytes[4];
            ipBytes[0] = ti & 0xFF;
            ipBytes[1] = (ti >> 8) & 0xFF;
            ipBytes[2] = (ti >> 16) & 0xFF;
            ipBytes[3] = (ti >> 24) & 0xFF;
            std::stringstream ti_printstream;
            ti_printstream << int(ipBytes[3]) << "." << int(ipBytes[2]) << "." << int(ipBytes[1]) << "."
                           << int(ipBytes[0]);
            std::cout << "Socket " << i << ": ";
            std::cout << " theirIP = " << ti_printstream.str();
            std::cout << " theirPort = " << tp;
            std::cout <<" myPort = " << mp << std::endl;
        }
        
    }
}

void KernelNetworklayer::populateSocketTable(std::vector<SocketType>& p_socketTable) {
    uint32_t theirIP_offset = udp_theirIP_offset;
    uint16_t theirPort_offset = udp_theirPort_offset;
    uint16_t myPort_offset = udp_myPort_offset;
    uint16_t valid_offset = udp_valid_offset;

    int numSocketsHW = this->readReg(udp_number_sockets);

    int l_socketTBsize = p_socketTable.size();
    if (numSocketsHW < l_socketTBsize) {
        std::string errMsg = "Socket list length " + std::to_string(l_socketTBsize) +
                             " is bigger than the number of sockets in hardware " + std::to_string(numSocketsHW);
        throw xansInvalidValue(errMsg);
    }

    for (int i = 0; i < l_socketTBsize; i++) {
        uint32_t ti_offset = theirIP_offset + i * 8;
        uint32_t tp_offset = theirPort_offset + i * 8;
        uint32_t mp_offset = myPort_offset + i * 8;
        uint32_t v_offset = valid_offset + i * 8;

        uint32_t theirIP = p_socketTable[i].theirIPint;
        this->writeReg(ti_offset, theirIP);
        this->writeReg(tp_offset, p_socketTable[i].theirPort);
        this->writeReg(mp_offset, p_socketTable[i].myPort);
        this->writeReg(v_offset, p_socketTable[i].valid);
    }
}

uint32_t  KernelNetworklayer::getUDPinPkts(){
    uint32_t l_res = this->readReg(udp_in_packets);
    uint32_t l_bytes = this->readReg(udp_in_bytes);
    std::cout << "udp in bytes = " << l_bytes << std::endl;
    return l_res;
}
uint32_t  KernelNetworklayer::getUDPoutPkts(){
    uint32_t l_res = this->readReg(udp_out_packets);
    uint32_t l_bytes = this->readReg(udp_out_bytes);
    std::cout << "udp out bytes = " << l_bytes << std::endl;
    l_bytes = this->readReg(ethhi_out_bytes);
    std::cout << "ethhi_out_bytes = " << l_bytes << std::endl;
    l_bytes = this->readReg(eth_out_bytes);
    std::cout << "eth_out_bytes = " << l_bytes << std::endl;
    return l_res;
}
uint32_t  KernelNetworklayer::getUDPappInPkts(){
    uint32_t l_res = this->readReg(udp_app_in_packets);
    uint32_t l_bytes = this->readReg(udp_app_in_bytes);
    std::cout << "udp app in bytes = " << l_bytes << std::endl;
    return l_res;
}
uint32_t  KernelNetworklayer::getUDPappOutPkts(){
    uint32_t l_res = this->readReg(udp_app_out_packets);
    uint32_t l_bytes = this->readReg(udp_app_out_bytes);
    std::cout << "udp app out bytes = " << l_bytes << std::endl;
    return l_res;
}

}
}

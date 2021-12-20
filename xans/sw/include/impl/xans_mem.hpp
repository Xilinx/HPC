
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

#ifndef XANS_MEM_HPP
#define XANS_MEM_HPP

#include "impl/cmac.hpp"
#include "impl/networklayer.hpp"
#include "impl/xnikMemHost.hpp"
#include "impl/xansException.hpp"

namespace xilinx_apps {
namespace xans {

template <unsigned int t_numInfs, unsigned int t_CmdBytes=14>
class XansImp {
   public:
    XansImp(){};
    void init(xilinx_apps::hpc_common::FPGA* p_fpga = nullptr) {
        for (unsigned int i = 0; i < t_numInfs; ++i) {
            m_cmacs[i].fpga(p_fpga);
            m_cmacs[i].initCU(i);
            m_networklayers[i].fpga(p_fpga);
            m_networklayers[i].initCU(i);
            m_xniks[i].fpga(p_fpga);
        }
    }
    void initXnik(xilinx_apps::hpc_common::FPGA* p_fpga, const unsigned int p_id) {
        for (unsigned int i = 0; i < t_numInfs; ++i) {
            m_xniks[i].fpga(p_fpga, p_id);
        }
    }
    void initNetXnik(xilinx_apps::hpc_common::FPGA* p_fpga, const unsigned int p_id) {
        for (unsigned int i = 0; i < t_numInfs; ++i) {
            m_networklayers[i].fpga(p_fpga);
            m_networklayers[i].initCU(p_id);
            m_xniks[i].fpga(p_fpga, p_id);
        }
    }
    std::map<std::string, bool> linkStatus(const unsigned int p_id) { return m_cmacs[p_id].linkStatus(); }
    void updateIpAddress(const unsigned int p_id, const std::string p_ipAddrStr) {
        uint32_t l_ipAddr = m_networklayers[p_id].updateIPAddress(p_ipAddrStr);
        m_xniks[p_id].setIPaddr(l_ipAddr);
        if (m_ipMap.find(l_ipAddr) == m_ipMap.end()) {
            m_ipMap.insert({l_ipAddr, p_id});
        }
        else {
            m_ipMap[l_ipAddr] = p_id;
        }        
    }
    void updateXnikIpAddress(const unsigned int p_id, const uint32_t p_ipAddr) {
        m_xniks[p_id].setIPaddr(p_ipAddr);
        if (m_ipMap.find(p_ipAddr) == m_ipMap.end()) {
            m_ipMap.insert({p_ipAddr, p_id});
        }
        else {
            m_ipMap[p_ipAddr] = p_id;
        }        
    }
    void updateIpAddress(const unsigned int p_id, const uint32_t p_ipAddr) {
        m_networklayers[p_id].updateIPAddress(p_ipAddr);
        m_xniks[p_id].setIPaddr(p_ipAddr);
        if (m_ipMap.find(p_ipAddr) == m_ipMap.end()) {
            m_ipMap.insert({p_ipAddr, p_id});
        }
        else {
            m_ipMap[p_ipAddr] = p_id;
        }        
    }

    void arpDiscovery(const unsigned int p_id) { m_networklayers[p_id].arpDiscovery(); }

    std::map<int, std::pair<std::string, std::string> > readARPTable(const unsigned int p_id, int num_entries = 256) {
        if (num_entries < 0) {
            throw xansInvalidValue("num_entries is smaller than zero");
        } else if (num_entries > 256) {
            throw xansInvalidValue("num_entries is greater than 256");
        }
        return m_networklayers[p_id].readARPTable(num_entries);
    }

    void configureSocket(const unsigned int p_id,
                         int index,
                         std::string p_theirIP,
                         uint16_t p_theirPort,
                         uint16_t p_myPort,
                         bool p_valid) {
        m_networklayers[p_id].configureSocket(index, p_theirIP, p_theirPort, p_myPort, p_valid);
    }

    SocketType getHostSocket(const unsigned int p_id, int index) { return m_networklayers[p_id].getHostSocket(index); }

    std::map<int, xilinx_apps::xans::SocketType> populateSocketTable(const unsigned int p_id) {
        return m_networklayers[p_id].populateSocketTable();
    }
    
    void populateSocketTable(const unsigned int p_id, std::vector<SocketType>& p_socketTable){
      m_networklayers[p_id].populateSocketTable(p_socketTable);
      m_networklayers[p_id].printSocketTable(p_socketTable.size());
    }

    void updateSockets(const uint32_t p_myIP) {
        uint16_t l_netInfId = getInfId(p_myIP);
        std::vector<SocketType> l_sockets = m_xniks[l_netInfId].getSockets();
        m_networklayers[l_netInfId].populateSocketTable(l_sockets);
        unsigned int l_numSockets = m_xniks[l_netInfId].getNumSockets();
        m_networklayers[l_netInfId].printSocketTable(l_numSockets);
    }
    void setXnikSockets(const uint32_t p_myIP, const std::vector<uint32_t>& p_sockets) {
        uint16_t l_netInfId = getInfId(p_myIP);
         m_xniks[l_netInfId].setSockets(p_sockets);
    }
    void addInstMPI(const uint32_t p_myIP, const uint32_t p_theirIP, const uint8_t p_opCode, const uint64_t p_bytes) {
        uint16_t l_netInfId = getInfId(p_myIP);
        m_xniks[l_netInfId].addInstMPI(p_opCode, p_bytes, p_theirIP);
    }
    void addInstCtl(const uint32_t p_myIP, const uint8_t p_opCode) {
        uint16_t l_netInfId = getInfId(p_myIP);
        m_xniks[l_netInfId].addInstCtl(p_opCode);
    }

    uint16_t getInfId(const uint32_t p_ipAddr) {
        if (m_ipMap.find(p_ipAddr) == m_ipMap.end()) {
            throw("Alveo network interface IP address does not match the given address "+getIPstr(p_ipAddr));
        }
        return m_ipMap[p_ipAddr]; 
    }
   
    void setXnikMem(const uint32_t p_myIP){
        uint16_t l_netInfId = getInfId(p_myIP);
        m_xniks[l_netInfId].setInstrMem();
    } 
    void* getXnikMem(const uint32_t p_myIP) {
        uint16_t l_netInfId = getInfId(p_myIP);
        return (m_xniks[l_netInfId].getInstMem());
    }
    unsigned int getXnikMemBytes(const uint32_t p_myIP) {
        uint16_t l_netInfId = getInfId(p_myIP);
        return (m_xniks[l_netInfId].getInstMemBytes());
    }
    void startXnik(const uint32_t p_myIP) {
        uint16_t l_netInfId = getInfId(p_myIP);
        m_xniks[l_netInfId].startXnik();
    }
    void finish() {
        for (unsigned int i = 0; i < t_numInfs; ++i) {
            m_xniks[i].finish();
        }
    }
    uint32_t getUDPinPkts(const uint16_t p_infId) {
        return (m_networklayers[p_infId].getUDPinPkts());
    } 
    uint32_t getUDPoutPkts(const uint16_t p_infId) {
        return (m_networklayers[p_infId].getUDPoutPkts());
    } 
    uint32_t getUDPappInPkts(const uint16_t p_infId) {
        return (m_networklayers[p_infId].getUDPappInPkts());
    } 
    uint32_t getUDPappOutPkts(const uint16_t p_infId) {
        return (m_networklayers[p_infId].getUDPappOutPkts());
    } 

   private:
    KernelCMAC m_cmacs[t_numInfs];
    KernelNetworklayer m_networklayers[t_numInfs];
    XnikMemHost<t_CmdBytes> m_xniks[t_numInfs];
    std::map<uint32_t, uint8_t> m_ipMap; //map ip address of xnik to the infId;
};
}
}

#endif

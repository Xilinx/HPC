
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

#ifndef XNIKHOST_HPP
#define XNIKHOST_HPP

#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "impl/networklayer.hpp"
#include "impl/xansException.hpp"
#include "mpiDefs.hpp"

namespace xilinx_apps {
namespace xans {

template <unsigned int t_CmdBytes, unsigned int t_NetDataBits=512>
class XnikHost {
    public:
        static const unsigned int t_NetDataBytes = t_NetDataBits/8;
    public:
        XnikHost() {
            m_numCmds = 0;
            m_numSockets = 1;
            SocketType l_socket;
            m_sockets.push_back(l_socket);
        }
        void setIPaddr(const uint32_t p_ipAddr) {
            m_ip = p_ipAddr;
        }
        uint32_t getIpAddr() {
            return m_ip;
        }
        std::vector<std::vector<uint8_t> >& getCmds() {
            return m_cmds;
        }
        unsigned int getNumCmds() {
            return m_numCmds;
        }
        unsigned int getNumSockets() {
            return m_numSockets;
        }

        void addMPIcmd(const uint8_t p_opCode, const uint64_t p_bytes, const uint16_t p_socketIdx) {
            std::vector<uint8_t> l_cmd(t_CmdBytes);
            l_cmd[0] = MPI_PKT_TYPE::CTRL;
            l_cmd[1] = p_opCode;
            l_cmd[2] = 0;
            l_cmd[3] = MPI_MSG_TAG::CTRL_NORM;
            uint64_t l_bytes = p_bytes;
            uint16_t l_socketIdx = p_socketIdx;
            memcpy(reinterpret_cast<uint8_t*>(&(l_cmd[4])), reinterpret_cast<uint8_t*>(&l_bytes), sizeof(p_bytes));
            memcpy(reinterpret_cast<uint8_t*>(&(l_cmd[12])), reinterpret_cast<uint8_t*>(&l_socketIdx), sizeof(p_socketIdx));
            m_numCmds++;
            m_cmds.push_back(l_cmd);
        }
        void addCmd(const uint8_t p_opCode){
            std::vector<uint8_t> l_cmd(t_CmdBytes);
            l_cmd[0] = p_opCode;
            m_numCmds++;
            m_cmds.push_back(l_cmd);
        }
        uint16_t setSocket(const uint32_t p_theirIPaddr) {
             uint16_t l_socketIdx;
            if (m_ipSocketMap.find(p_theirIPaddr) == m_ipSocketMap.end() ) {
                SocketType l_socket;
                l_socket.theirIPint = p_theirIPaddr;
                l_socket.theirIP = getIPstr(p_theirIPaddr);
                l_socket.valid = true;
                m_ipSocketMap.insert({p_theirIPaddr, m_numSockets});
                l_socketIdx = m_numSockets;
                m_sockets.push_back(l_socket);
                m_numSockets++;
            }
            else {
                l_socketIdx = m_ipSocketMap[p_theirIPaddr];
                m_sockets[l_socketIdx].theirIPint = p_theirIPaddr;
                m_sockets[l_socketIdx].theirIP = getIPstr(p_theirIPaddr);
                m_sockets[l_socketIdx].valid = true;
            }
            return l_socketIdx;
        }
        void updateSocket0(const uint32_t p_cpuIP) {
            m_sockets[0].theirIPint = p_cpuIP;
            m_sockets[0].theirIP = getIPstr(p_cpuIP);
            m_sockets[0].valid = true;
            if (m_ipSocketMap.find(p_cpuIP) == m_ipSocketMap.end()) {
                m_ipSocketMap.insert({p_cpuIP, 0});
            }
            else {
                 m_ipSocketMap[p_cpuIP] = 0;
            }           
        }

        std::vector<SocketType>& getSockets() {
            return m_sockets;
        }
        
        void sendCmds() {
            int l_socket, l_ret;
            struct sockaddr_in l_srcSockAddr;
            l_socket = socket(AF_INET, SOCK_DGRAM, 0);
            if (l_socket < 0) {
                throw xansInvalidValue("failed to create UDP socket.");
            }
            memset(&l_srcSockAddr, 0, sizeof(l_srcSockAddr));     
            l_srcSockAddr.sin_family = AF_INET;
            l_srcSockAddr.sin_addr.s_addr = inet_addr(m_sockets[0].theirIP.c_str());
            unsigned short l_srcPort = (unsigned short)m_sockets[0].theirPort;
            l_srcSockAddr.sin_port = htons(l_srcPort);
            l_ret = bind(l_socket, (struct sockaddr*)&l_srcSockAddr, sizeof(l_srcSockAddr));
            if (l_ret < 0) {
                throw xansInvalidValue("socket bind failed.");
            }

            struct sockaddr_in l_myAddr;
            memset(&l_myAddr, 0, sizeof(l_myAddr));
            l_myAddr.sin_family = AF_INET;
            l_myAddr.sin_addr.s_addr = inet_addr(getIPstr(m_ip).c_str());
            unsigned short l_myPort = (unsigned short)m_sockets[0].myPort; 
            l_myAddr.sin_port = htons(l_myPort);

            //send cmds from CPU to alveo
            std::vector<uint8_t> l_datPkt(t_NetDataBytes);
            for (unsigned int i=0; i<m_numCmds; ++i) {
                memcpy(l_datPkt.data(), m_cmds[i].data(), t_NetDataBytes);
                l_ret = sendto(l_socket, (const char*)(l_datPkt.data()), t_NetDataBytes, 0,
                            (const struct sockaddr*)&l_myAddr, sizeof(l_myAddr));    
                if (l_ret < 0) {
                    throw xansInvalidValue("faile to send cmd to Alveo card.");
                }
            }
            close(l_socket);
        }
        
    private:
        std::vector<std::vector<uint8_t> > m_cmds;
        unsigned int m_numCmds;
        uint32_t m_ip;
        std::vector<SocketType> m_sockets;
        uint16_t m_numSockets;
        std::map<uint32_t, uint16_t> m_ipSocketMap; //map their IP address to socket idx
};

}
}

#endif

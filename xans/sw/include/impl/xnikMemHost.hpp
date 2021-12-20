
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

#ifndef XNIKMEMHOST_HPP
#define XNIKMEMHOST_HPP

#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "xnikInstr.hpp"
#include "sw/xNativeFPGA.hpp"
#include "impl/networklayer.hpp"
#include "impl/xansException.hpp"
#include "mpiDefs.hpp"

namespace xilinx_apps {
namespace xans {

template <unsigned int t_CmdBytes, unsigned int t_NetDataBits=512>
class XnikMemHost {
    public:
        static const unsigned int t_NetDataBytes = t_NetDataBits/8;
    public:
        XnikMemHost() {
            m_numSockets = 0;
            m_numInstrs = 0;
            m_instrs.resize(t_CmdBytes);
        }
        void fpga(hpc_common::FPGA* p_fpga, const unsigned int p_id=0) {
            m_krnXnik.fpga(p_fpga);
            std::string l_cuName = "krnl_xnik:{krnl_xnik_"+std::to_string(p_id)+"}";
            m_krnXnik.createKernel(l_cuName);
        }
        void setIPaddr(const uint32_t p_ipAddr) {
            m_ip = p_ipAddr;
        }
        uint32_t getIpAddr() {
            return m_ip;
        }
        unsigned int getNumSockets() {
            return m_numSockets;
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

        void setSockets(const std::vector<uint32_t>& p_sockets) {
            unsigned int l_size = p_sockets.size();
            m_numSockets = l_size;
            m_sockets.resize(l_size);
            for (unsigned int i=0; i<l_size; ++i) {
                uint32_t l_ipAddr = p_sockets[i];
                m_ipSocketMap[l_ipAddr] = i;
                m_sockets[i].theirIPint = l_ipAddr;
                m_sockets[i].theirIP = getIPstr(l_ipAddr);
                m_sockets[i].valid = true;
            }
        }

        std::vector<SocketType>& getSockets() {
            return m_sockets;
        }
        
        void addInstMPI(const uint8_t p_opCode, const uint64_t p_bytes, const uint32_t p_theirIP) {
            m_numInstrs++;
            uint16_t l_socketIdx;
            if (m_ipSocketMap.find(p_theirIP) == m_ipSocketMap.end() ) {
                l_socketIdx = setSocket(p_theirIP);
            }
            else {
                l_socketIdx = m_ipSocketMap[p_theirIP];
            }
            XNIKInstr<t_CmdBytes> l_instr;
            l_instr.setOpCode(p_opCode);
            l_instr.setBufAddr(0);
            l_instr.setMsgTag(MPI_MSG_TAG::CTRL_NORM); 
            l_instr.setLength(p_bytes);
            l_instr.setSocketIdx(l_socketIdx);
            xf::hpc::MemInstr<t_CmdBytes> l_memInstr;
            l_instr.encode(l_memInstr);
            for (unsigned int i=0; i<t_CmdBytes; ++i) {
                 m_instrs.push_back(l_memInstr[i]);
            }
        }
        void addInstCtl(const uint8_t p_opCode) {
            m_numInstrs++;
            XNIKInstr<t_CmdBytes> l_instr;
            l_instr.setOpCode(p_opCode);
            xf::hpc::MemInstr<t_CmdBytes> l_memInstr;
            l_instr.encode(l_memInstr);
            for (unsigned int i=0; i<t_CmdBytes; ++i) {
                 m_instrs.push_back(l_memInstr[i]);
            }
        }
        void setInstrMem() {
            m_instrs[0] = m_numInstrs & 0xff;
            m_instrs[1] = (m_numInstrs >> 8) & 0xff;
            size_t l_bytes = m_instrs.size();
            if (l_bytes % t_CmdBytes != 0) {
                throw xansInvalidValue("kernel xnik buffer size must be multiple of " + std::to_string(t_CmdBytes));
            }
            void* l_outBuf = m_krnXnik.createBO(0, l_bytes);
            memcpy(l_outBuf, m_instrs.data(), l_bytes);
            m_krnXnik.setMemArg(0);
            m_krnXnikBufs.insert({0, l_outBuf});
            m_krnXnik.sendBO(0);
        }
        void* getInstMem() {
            return m_instrs.data();
        }
        unsigned int getInstMemBytes() {
            return m_instrs.size();
        }
        void startXnik() {
            m_krnXnik.run();
        }
        void finish() {
            m_krnXnik.wait();
            m_krnXnikBufs.clear();
            m_instrs.clear();
            m_numInstrs=0;
        }
    private:
        hpc_common::KERNEL m_krnXnik;
        std::map<const int, void*> m_krnXnikBufs;
        std::vector<uint8_t> m_instrs;
        uint16_t m_numInstrs;
        uint32_t m_ip;
        std::vector<SocketType> m_sockets;
        uint16_t m_numSockets;
        std::map<uint32_t, uint16_t> m_ipSocketMap; //map their IP address to socket idx
};

}
}

#endif

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

#ifndef XNIKINSTR_HPP
#define XNIKINSTR_HPP

#include "memInstr.hpp"
#include "mpiDefs.hpp"

namespace xilinx_apps {
namespace xans {


template <unsigned int t_InstrBytes>
class XNIKInstr {
public:
    XNIKInstr() {}
    void setOpCode(const uint8_t p_opCode) {
#pragma HLS INLINE
        m_opCode = p_opCode;
    }
    uint8_t getOpCode() const {
#pragma HLS INLINE
        return m_opCode;
    }
    void setMsgTag(const uint8_t p_msgTag) {
#pragma HLS INLINE
        m_msgTag = p_msgTag;
    }
    uint8_t getMsgTag() const {
#pragma HLS INLINE
        return m_msgTag;
    }
    void setBufAddr(const uint8_t p_bufAddr) {
#pragma HLS INLINE
        m_bufAddr = p_bufAddr;
    }
    uint8_t getBufAddr() const {
#pragma HLS INLINE
        return m_bufAddr;
    }
    void setSocketIdx(const uint16_t p_socketIdx) {
#pragma HLS INLINE
        m_socketIdx = p_socketIdx;
    }
    uint16_t getSocketIdx() const {
#pragma HLS INLINE
        return m_socketIdx;
    }
    void setLength(const uint64_t p_length) {
#pragma HLS INLINE
        m_length = p_length;
    }
    uint64_t getLength() const {
#pragma HLS INLINE
        return m_length;
    }
    template <typename T>
    void store(T* p_mem, xf::hpc::MemInstr<t_InstrBytes>& p_memInstr) {
#pragma HLS INLINE
        encode(p_memInstr);
        p_memInstr.template store<T>(p_mem);
    }
    template <typename T>
    void load(T* p_mem, xf::hpc::MemInstr<t_InstrBytes>& p_memInstr) {
#pragma HLS INLINE
        p_memInstr.template load<T>(p_mem);
        decode(p_memInstr);
    }

#ifndef __SYNTHESIS__
    friend std::ostream& operator<<(std::ostream& os, XNIKInstr& p_instr) {
        os << "m_opCode: " << p_instr.m_opCode << ",";
        os << "m_msgTag: " << p_instr.m_msgTag << ",";
        os << "m_bufAddr: " << p_instr.m_bufAddr << ",";
        os << "m_socketIdx: " << p_instr.m_socketIdx << ",";
        os << "m_length: " << p_instr.m_length <<",";
        return os;
    }
#endif

    void encode(xf::hpc::MemInstr<t_InstrBytes>& p_memInstr) {
#pragma HLS INLINE
        unsigned int l_loc = 0;
        p_memInstr.encode(l_loc, m_opCode);
        p_memInstr.encode(l_loc, m_msgTag);
        p_memInstr.encode(l_loc, m_bufAddr);
        p_memInstr.encode(l_loc, m_socketIdx);
        p_memInstr.encode(l_loc, m_length);
    }

    void decode(xf::hpc::MemInstr<t_InstrBytes>& p_memInstr) {
#pragma HLS INLINE
        unsigned int l_loc = 0;
        p_memInstr.decode(l_loc, m_opCode);
        p_memInstr.decode(l_loc, m_msgTag);
        p_memInstr.decode(l_loc, m_bufAddr);
        p_memInstr.decode(l_loc, m_socketIdx);
        p_memInstr.decode(l_loc, m_length);
    }

private:
    uint8_t m_opCode;//0: mpiSend, 1: mpiRec
    uint8_t m_msgTag;
    uint8_t m_bufAddr;
    uint16_t m_socketIdx;    
    uint64_t m_length;
};

}
}
#endif

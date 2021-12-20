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

#ifndef PKTDEFS_HPP
#define PKTDEFS_HPP

#include "ap_axi_sdata.h"
#include "ap_int.h"
#include "hls_stream.h"
#include "mpiDefs.hpp"

namespace xilinx_apps {
namespace xans {

template <unsigned int t_DataBits,
          unsigned int t_UserBits,
          unsigned int t_DestBits>
class PktUDP {
    public:
        typedef ap_axiu<t_DataBits, t_UserBits, 1, t_DestBits> TypeAXIS;
    public:
        PktUDP() {}
        void setPkt(const TypeAXIS& p_pkt) {
#pragma HLS INLINE
            m_pkt = p_pkt;
        }
        void reset() {
            m_pkt.keep = -1;
            m_pkt.last = 0;
        }
        void readPktStream(hls::stream<TypeAXIS>& p_str) {
            p_str.read(m_pkt);
        }
        void writePktStrem(hls::stream<TypeAXIS>& p_str) {
            p_str.write(m_pkt);
        }
        TypeAXIS& getPkt() {
            return m_pkt;
        }
        void setDat(ap_uint<t_DataBits>& p_dat) {
            m_pkt.data = p_dat;
        }
        void setLast() {
            m_pkt.last = 1;
        }
    protected:
        TypeAXIS m_pkt;
};

template <unsigned int t_DataBits,
          unsigned int t_UserBits,
          unsigned int t_DestBits>
class PktXNIK : public PktUDP<t_DataBits,
                              t_UserBits,
                              t_DestBits> {
public:
    PktXNIK() {
    }
    void update() {
#pragma HLS INLINE
        m_type = this->m_pkt.data(7,0);
        m_opCode = this->m_pkt.data(15,8);
        m_bufAddr = this->m_pkt.data(23,16);
        m_msgTag = this->m_pkt.data(31,24);
        m_length = this->m_pkt.data(95,32);
        m_socketIdx = this->m_pkt.data(111,96);
    }
    bool isCtrl() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::CTRL));
    }
    bool isSync() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::SYNC));
    }
    bool isAck() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::ACK));
    }
    bool isFinal() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::FIN));
    }
    bool isIntCtrl() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::INT_CTRL));
    }
    bool isLast() {
#pragma HLS INLINE
        return ((this->m_pkt.last == 1) && (m_type == MPI_PKT_TYPE::LAST));
    }
    bool isData() {
#pragma HLS INLINE
        return (m_type == MPI_PKT_TYPE::DATA);
    }
    ap_uint<8> getType() const {
#pragma HLS INLINE
        return m_type;
    }
    ap_uint<8> getOpCode() const {
#pragma HLS INLINE
        return m_opCode;
    }
    ap_uint<8> getBufAddr() const {
#pragma HLS INLINE
        return m_bufAddr;
    }
    ap_uint<8> getMsgTag() const {
#pragma HLS INLINE
        return m_msgTag;
    }
    ap_uint<64> getLength() const {
#pragma HLS INLINE
        return m_length;
    }
    ap_uint<16> getSocketIdx() const {
#pragma HLS INLINE
        return m_socketIdx;
    }
    void setSynPkt() {
#pragma HLS INLINE
        this->m_pkt.data(7,0) = MPI_PKT_TYPE::SYNC;
    }
    void setAckPkt() {
#pragma HLS INLINE
        this->m_pkt.data(7,0) = MPI_PKT_TYPE::ACK;
    }
    void setDatPkt(ap_uint<t_DataBits>& p_dat) {
#pragma HLS INLINE
        this->m_pkt.data(t_DataBits-1, 0) = p_dat;
    }
private:
    ap_uint<8> m_type;
    ap_uint<8> m_opCode;
    ap_uint<8> m_bufAddr;
    ap_uint<8> m_msgTag;
    ap_uint<64> m_length;
    ap_uint<16> m_socketIdx;
};


}
}

#endif

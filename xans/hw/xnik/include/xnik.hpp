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

#ifndef XNIK_HPP
#define XNIK_HPP

#include "mpiDefs.hpp"
#include "pktDefs.hpp"
#include "xnikInstr.hpp"
#include "xf_blas.hpp"

#ifndef __SYNTHESIS__
#include <cassert>
#endif

namespace xilinx_apps {
namespace xans {
//data transfer length is always aligned to t_NetDataBytes;
template <unsigned int t_MTUBytes,
          unsigned int t_MaxInstrs,
          unsigned int t_NetDataBits,
          unsigned int t_UserBits,
          unsigned int t_DestBits,
          unsigned int t_MemBits>
class XNIK {
public:
    static constexpr unsigned int t_NetDataBytes = t_NetDataBits/8;
    static constexpr unsigned int t_MemDataInt16s = t_MemBits/8/sizeof(uint16_t);
    static constexpr unsigned int t_MemDataBytes = t_MemBits/8;
    static constexpr unsigned int t_MaxMsgSize = t_MTUBytes / t_NetDataBytes;
    typedef typename PktUDP<t_NetDataBits, t_UserBits, t_DestBits>::TypeAXIS PktType;
public:
    XNIK(){}
    void decodePkt(ap_uint<t_MemBits>* p_memPtr,
                 hls::stream<ap_uint<1> >& p_inLoopExitStr,
                hls::stream<ap_uint<3> >& p_outScheduleStr,
                 hls::stream<ap_uint<t_DestBits> >& p_outDest2SendStr,
                 hls::stream<ap_uint<8> >& p_outCtrl2SendStr,
                 hls::stream<ap_uint<t_DestBits> >& p_outDest2RecStr,
                 hls::stream<ap_uint<8> >& p_outCtrl2RecStr,
                 hls::stream<ap_uint<64> >& p_outLen2RecStr) {
//schedule string deciding
        //bit 2: 1=read receiver string, 0=do not read receiver string
        //bit 1: 1=read sender string, 0=do not read sender string
        //bit 0: 1=read receiver string first, 0=read sender string first
        uint16_t l_totalInstrs = 0;
        uint16_t l_loopStartIdx = 0;
        uint16_t l_loopEndIdx = 0;
        uint16_t l_loopSize = 0;
        xf::blas::WideType<uint16_t, t_MemDataInt16s> l_val = p_memPtr[0];
#pragma HLS ARRAY_PARTITION variable=l_val complete dim=1
        l_totalInstrs = l_val[0];
        uint16_t l_pc = 1;
        while (l_pc <= l_totalInstrs) {
            xf::hpc::MemInstr<t_MemDataBytes> l_memInstr = p_memPtr[l_pc];
            XNIKInstr<t_MemDataBytes> l_instr;
            l_instr.decode(l_memInstr);
            uint8_t l_opCode = l_instr.getOpCode();
            uint8_t l_msgTag = l_instr.getMsgTag();
            uint16_t l_dest = l_instr.getSocketIdx();
            uint64_t l_lenNetWords = l_instr.getLength() / t_NetDataBytes;
            PktType l_pkt;
            if ((l_opCode == MPI_OPCODE::MPI_SEND) && (l_msgTag == MPI_MSG_TAG::CTRL_NORM)) {
                p_outScheduleStr.write(6); // 110 (bit 2,1,0=110,meaning merger reading sender first, and then read receiver)
                p_outDest2SendStr.write(l_dest);
                p_outCtrl2SendStr.write(MPI_MSG_TAG::CTRL_SYNC); //ask Sender to send SYNC packet
                p_outDest2RecStr.write(l_dest);
                p_outCtrl2RecStr.write(MPI_MSG_TAG::CTRL_SEND);
                p_outLen2RecStr.write(l_lenNetWords);
            }
            else if ((l_opCode == MPI_OPCODE::MPI_RECEIVE) && (l_msgTag == MPI_MSG_TAG::CTRL_NORM)) {
                p_outScheduleStr.write(5);
                p_outScheduleStr.write(5);
                p_outDest2RecStr.write(l_dest);
                p_outCtrl2RecStr.write(MPI_MSG_TAG::CTRL_RECEIVE); 
                p_outLen2RecStr.write(l_lenNetWords);
            }
            if (l_msgTag == MPI_MSG_TAG::CTRL_LOOP_START) {
                l_loopStartIdx = l_pc;
            }
            else if (l_msgTag == MPI_MSG_TAG::CTRL_LOOP_END) {
                l_loopEndIdx = l_pc;
                l_pc = l_loopStartIdx;
            }
            else {
                l_pc++;
            }
        }
        p_outScheduleStr.write(5);
        p_outCtrl2RecStr.write(MPI_MSG_TAG::CTRL_FIN);
        p_outDest2RecStr.write(0); 
        p_outLen2RecStr.write(0);
    }
                   
    void recDat(hls::stream<PktType>& p_inPktStr,
                hls::stream<ap_uint<t_DestBits> >& p_inDestStr,
                hls::stream<ap_uint<8> >& p_inCtrlStr,
                hls::stream<ap_uint<64> >& p_inLenStr,
                hls::stream<ap_uint<t_DestBits> >& p_outDestStr,
                hls::stream<ap_uint<8> >& p_outCtrlStr,
                hls::stream<ap_uint<64> >& p_outLenStr,
                hls::stream<ap_uint<t_NetDataBits> >& p_outDatStr) {
        ap_uint<8> l_tagType = p_inCtrlStr.read();
        ap_uint<t_DestBits> l_dest = p_inDestStr.read();
        ap_uint<64> l_len = p_inLenStr.read();
        PktType l_pkt;
        while (l_tagType != MPI_MSG_TAG::CTRL_FIN) {
            if (l_tagType == MPI_MSG_TAG::CTRL_SEND) {
                l_pkt = p_inPktStr.read();
                if (l_pkt.data(7,0) == MPI_PKT_TYPE::ACK) {
                    p_outDestStr.write(l_dest);
                    p_outCtrlStr.write(l_tagType);
                    p_outLenStr.write(l_len);
                }
            }
            if (l_tagType == MPI_MSG_TAG::CTRL_RECEIVE) {
                l_pkt = p_inPktStr.read();
                if (l_pkt.data(7,0) == MPI_PKT_TYPE::SYNC) {
                    p_outDestStr.write(l_dest);
                    p_outCtrlStr.write(MPI_MSG_TAG::CTRL_ACK);
                    p_outLenStr.write(0);
                }
                for (unsigned int i=0; i<l_len; ++i) {
#pragma HLS PIPELINE II=1
                    l_pkt = p_inPktStr.read();
                    p_outDatStr.write(l_pkt.data);
                }
                p_outDestStr.write(l_dest);
                p_outCtrlStr.write(MPI_MSG_TAG::CTRL_RECEIVE);
                p_outLenStr.write(0);
            } 
            l_tagType = p_inCtrlStr.read();
            l_dest = p_inDestStr.read();
            l_len = p_inLenStr.read();
        }
        p_outCtrlStr.write(MPI_MSG_TAG::CTRL_FIN);
        p_outDestStr.write(0); 
        p_outLenStr.write(0);
    }

    void mergeStrs(hls::stream<ap_uint<3> >& p_scheduleStr,
                   hls::stream<ap_uint<t_DestBits> >& p_destFromDecStr,
                   hls::stream<ap_uint<8> >& p_ctrlFromDecStr,
                   hls::stream<ap_uint<t_DestBits> >& p_destFromRecStr,
                   hls::stream<ap_uint<8> >& p_ctrlFromRecStr,
                   hls::stream<ap_uint<64> >& p_lenFromRecStr,
                   hls::stream<ap_uint<t_DestBits> >& p_destOutStr,
                   hls::stream<ap_uint<8> >& p_ctrlOutStr,
                   hls::stream<ap_uint<64> >& p_lenOutStr) {
        //because the dest stream output is directly connected to the AXIS, we have to use non-blocking read here. Otherwise, system will hang up.
        ap_uint<8> l_tagType = MPI_MSG_TAG::CTRL_NORM;
        ap_uint<t_DestBits> l_dest;
        ap_uint<64> l_len;
        ap_uint<3> l_sch = 0; 
        bool l_readDec = false;
        bool l_readRec = false;
        do {
            if (l_readDec && !l_readRec && (!p_ctrlFromDecStr.empty() || !p_destFromDecStr.empty())){
                l_tagType = p_ctrlFromDecStr.read();
                p_ctrlOutStr.write(l_tagType);
                l_dest = p_destFromDecStr.read();
                p_destOutStr.write(l_dest);
                l_readDec = !l_readDec;
                l_readRec = !l_sch[0] && l_sch[2];
            }
            if (l_readRec && !l_readDec && (!p_ctrlFromRecStr.empty() || ! p_destFromRecStr.empty() || !p_lenFromRecStr.empty()) ) {
                l_tagType = p_ctrlFromRecStr.read();
                p_ctrlOutStr.write(l_tagType);
                l_dest = p_destFromRecStr.read();
                p_destOutStr.write(l_dest);
                l_len = p_lenFromRecStr.read();
                if (l_len != 0) {
                    p_lenOutStr.write(l_len);
                }
                l_readRec = !l_readRec;
                l_readDec = l_sch[0] && l_sch[1];
            }
            if (!l_readDec && !l_readRec && !p_scheduleStr.empty()){
                l_sch = p_scheduleStr.read();
                l_readDec = (!l_sch[0]&& l_sch[1]);
                l_readRec = (l_sch[0] && l_sch[2]);
            }
            
        } while(l_tagType != MPI_MSG_TAG::CTRL_FIN);
        
    }

    void sendDat(hls::stream<ap_uint<t_DestBits> >& p_inDestStr,
                 hls::stream<ap_uint<8> >& p_inCtrlStr,
                 hls::stream<ap_uint<64> >& p_inLenStr,
                 hls::stream<ap_uint<t_NetDataBits> >& p_inDatStr,
                 hls::stream<PktType>& p_outPktStr) {
        PktType l_pkt;
        l_pkt.keep = -1;
        ap_uint<8> l_tagType = p_inCtrlStr.read();
        ap_uint<t_DestBits> l_dest = p_inDestStr.read();
        while (l_tagType != MPI_MSG_TAG::CTRL_FIN) {
            l_pkt.dest = l_dest;
            if (l_tagType == MPI_MSG_TAG::CTRL_SYNC) { //normal mpi_send for sending data
                //send SYNC packet
                l_pkt.data(7,0) =  MPI_PKT_TYPE::SYNC;
                l_pkt.last = 1;
                p_outPktStr.write(l_pkt);
            }
            if (l_tagType == MPI_MSG_TAG::CTRL_ACK) {
                l_pkt.data(7,0) = MPI_PKT_TYPE::ACK;
                l_pkt.last = 1;
                p_outPktStr.write(l_pkt);
            }
            if (l_tagType == MPI_MSG_TAG::CTRL_SEND) {
                uint64_t l_msgSize = p_inLenStr.read();
                uint8_t l_netWordCounter = 1;
                for (unsigned int i=1; i<=l_msgSize; ++i) {
#pragma HLS PIPELINE II=1
                    l_pkt.data = p_inDatStr.read();
                    if ((l_netWordCounter == t_MaxMsgSize) || (i == l_msgSize)) {
                        l_pkt.last = 1;
                        l_netWordCounter = 1;
                    }
                    else {
                        l_pkt.last = 0;
                        l_netWordCounter++;
                    }
                    p_outPktStr.write(l_pkt);
                } 
            }
            l_tagType = p_inCtrlStr.read();
            l_dest = p_inDestStr.read();
        }
    }

    void process_mem (ap_uint<t_MemBits>* p_memPtr,
                 hls::stream<PktType>& p_inPktStr,
                 hls::stream<ap_uint<1> >& p_inLoopExitStr,
                 hls::stream<ap_uint<t_NetDataBits> >& p_inDatStr,
                 hls::stream<PktType>& p_outPktStr,
                 hls::stream<ap_uint<t_NetDataBits> >& p_outDatStr) {
        hls::stream<ap_uint<3> > l_scheduleStr;
        hls::stream<ap_uint<t_DestBits> > l_dest2SendStr;
        hls::stream<ap_uint<t_DestBits> > l_dest2RecStr;
        hls::stream<ap_uint<t_DestBits> > l_destFromRecStr;
        hls::stream<ap_uint<t_DestBits> > l_destFromMergStr;
        hls::stream<ap_uint<8> > l_ctrl2SendStr;
        hls::stream<ap_uint<8> > l_ctrl2RecStr;
        hls::stream<ap_uint<8> > l_ctrlFromRecStr;
        hls::stream<ap_uint<8> > l_ctrlFromMergStr;
        hls::stream<ap_uint<64> > l_len2RecStr;
        hls::stream<ap_uint<64> > l_lenFromRecStr;
        hls::stream<ap_uint<64> > l_lenFromMergStr;
#pragma HLS STREAM variable=l_scheduleStr depth=4
#pragma HLS STREAM variable=l_dest2SendStr depth=4
#pragma HLS STREAM variable=l_dest2RecStr depth=4
#pragma HLS STREAM variable=l_destFromRecStr depth=4
#pragma HLS STREAM variable=l_destFromMergStr depth=4
#pragma HLS STREAM variable=l_ctrl2SendStr depth=4
#pragma HLS STREAM variable=l_ctrl2RecStr depth=4
#pragma HLS STREAM variable=l_ctrlFromRecStr depth=4
#pragma HLS STREAM variable=l_ctrlFromMergStr depth=4
#pragma HLS STREAM variable=l_len2RecStr depth=4
#pragma HLS STREAM variable=l_lenFromRecStr depth=4
#pragma HLS STREAM variable=l_lenFromMergStr depth=4
#pragma HLS DATAFLOW
        decodePkt(p_memPtr, p_inLoopExitStr, l_scheduleStr, l_dest2SendStr, l_ctrl2SendStr,
                 l_dest2RecStr, l_ctrl2RecStr, l_len2RecStr);
        recDat(p_inPktStr, l_dest2RecStr, l_ctrl2RecStr, l_len2RecStr,
               l_destFromRecStr, l_ctrlFromRecStr, l_lenFromRecStr, p_outDatStr);

        mergeStrs(l_scheduleStr, l_dest2SendStr, l_ctrl2SendStr,
                  l_destFromRecStr, l_ctrlFromRecStr, l_lenFromRecStr,
                  l_destFromMergStr, l_ctrlFromMergStr, l_lenFromMergStr); 
        sendDat(l_destFromMergStr, l_ctrlFromMergStr, l_lenFromMergStr, p_inDatStr, p_outPktStr);
    }
    
};

}
}

#endif

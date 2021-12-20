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
#include "hw/interface.hpp"
#include "xnik.hpp"

typedef xilinx_apps::xans::PktUDP<XANS_netDataBits, XANS_userBits, XANS_destBits>::TypeAXIS PktType;
extern "C" void krnl_xnik(hls::stream<PktType>& p_inPktStr,
             hls::stream<ap_uint<1> >& p_inLoopExitStr,
             hls::stream<ap_uint<XANS_netDataBits> >& p_inDatStr,
             hls::stream<PktType>& p_outPktStr,
             hls::stream<ap_uint<XANS_netDataBits> >& p_outDatStr) {
    AXIS(p_inPktStr);
    AXIS(p_inLoopExitStr);
    AXIS(p_inDatStr);
    AXIS(p_outPktStr);
    AXIS(p_outDatStr);
    AP_CTRL_NONE(return);
    xilinx_apps::xans::XNIK<XANS_mtuBytes,
                            XANS_instrBytes,
                            XANS_maxInstrs,
                            XANS_netDataBits,
                            XANS_userBits,
                            XANS_destBits> l_xnik;
    l_xnik.process(p_inPktStr,
                   p_inLoopExitStr,
                   p_inDatStr,
                   p_outPktStr,
                   p_outDatStr);
}

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

#include "interface.hpp"
#include "compute_alpha.hpp"
#include "krnl_alpha.hpp"

extern "C" void krnl_alpha(CG_interface* p_pk,
                               CG_interface* p_Apk,
                               hls::stream<ap_uint<CG_tkStrWidth> >& p_tokenIn,
                               hls::stream<ap_uint<CG_tkStrWidth> >& p_tokenOut) {
    POINTER(p_pk, gmem_pk)
    POINTER(p_Apk, gmem_Apk)
    AXIS(p_tokenIn)
    AXIS(p_tokenOut)
    SCALAR(return )

#pragma HLS DATAFLOW
    xf::hpc::cg::compute_alpha<CG_dataType, CG_vecParEntries, CG_tkStrWidth>(p_pk, p_Apk, p_tokenIn,
                                                                         p_tokenOut);
}

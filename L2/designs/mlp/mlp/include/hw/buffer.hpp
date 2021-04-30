/*
 * Copyright 2019 Xilinx, Inc.
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
#ifndef __XF_HPC_BUFFER_HPP__
#define __XF_HPC_BUFFER_HPP__
#include "xf_blas.hpp"
namespace xf {
namespace hpc {
template <typename t_DataType, unsigned int t_BufferSize>
class Buffer {
   public:
    static void buffer(unsigned int p_n,
                       unsigned int p_batch,
                       hls::stream<t_DataType>& p_in,
                       hls::stream<t_DataType>& p_out,
                       unsigned int p_reuseNum = 1) {
        if (p_n > t_BufferSize || p_reuseNum == 1) {
            for (int l_block = 0; l_block < p_batch * p_n * p_reuseNum; ++l_block) {
#pragma HLS PIPELINE
                p_out.write(p_in.read());
            }
        } else {
            t_DataType l_buffer[t_BufferSize];
            for (int l_block = 0; l_block < p_batch; ++l_block) {
                for (int i = 0; i < p_reuseNum; ++i) {
                    for (int l = 0; l < p_n; ++l) {
#pragma HLS PIPELINE
                        if (i == 0) {
                            l_buffer[l] = p_in.read();
                        }
                        t_DataType l_word = l_buffer[l];
                        p_out.write(l_word);
                    }
                }
            }
        }
    }

    static void readMem(unsigned int p_n,
                        t_DataType* p_mem,
                        hls::stream<t_DataType>& p_str,
                        unsigned int p_reuse = 1,
                        unsigned int p_batch = 1) {
#pragma HLS DATAFLOW
        hls::stream<t_DataType> l_str;
        xf::blas::mem2stream(p_n, p_mem, l_str, p_n > t_BufferSize ? p_reuse : 1, p_batch);
        buffer(p_n, p_batch, l_str, p_str, p_reuse);
    }
};
}
}
#endif

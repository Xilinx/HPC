/**********
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
 * **********/
#ifndef XF_HPC_DOUBLEBUFFER_HPP
#define XF_HPC_DOUBLEBUFFER_HPP
#include "buffer.hpp"
namespace xf {
namespace hpc {

template <typename t_DataType,
          unsigned int t_BufferSize> // number of memory words in one row of the matrix B buffer
class DoubleBuffer : public Buffer<t_DataType, t_BufferSize> {
   public:
    typedef hls::stream<t_DataType> DdrStream;
    typedef Buffer<t_DataType, t_BufferSize> t_BufferType;

    void writeMem(unsigned int p_n, hls::stream<t_DataType>& p_str, t_DataType* p_mem, unsigned int p_batch = 1) {
#pragma HLS DATAFLOW
        hls::stream<t_DataType> l_str;
        process(p_str, l_str, p_n, 1, p_batch);
        xf::blas::stream2mem(p_n, l_str, p_mem, p_batch);
    }

    void readMem(unsigned int p_n,
                 t_DataType* p_mem,
                 hls::stream<t_DataType>& p_str,
                 unsigned int p_reuse = 1,
                 unsigned int p_batch = 1) {
#pragma HLS DATAFLOW
        hls::stream<t_DataType> l_str;
        xf::blas::mem2stream(p_n, p_mem, l_str, p_n > t_BufferSize ? p_reuse : 1, p_batch);
        process(l_str, p_str, p_n, p_reuse, p_batch);
    }

    void process(DdrStream& p_streamIn,
                 DdrStream& p_streamOut,
                 unsigned int p_n,
                 unsigned int p_reuseNum = 1,
                 unsigned int p_batch = 1) {
        DdrStream p_s0_0, p_s0_1, p_s1_0, p_s1_1;
#pragma HLS DATAFLOW
        split(p_n, p_batch, p_streamIn, p_s0_0, p_s0_1);
        buffer_0.buffer(p_n, (p_batch / 2) + (p_batch % 2), p_s0_0, p_s1_0, p_reuseNum);
        buffer_1.buffer(p_n, (p_batch / 2), p_s0_1, p_s1_1, p_reuseNum);
        merge(p_n, p_batch, p_s1_0, p_s1_1, p_streamOut, p_reuseNum);
    }

    void split(unsigned int p_n, unsigned int p_batch, DdrStream& p_in, DdrStream& p_out1, DdrStream& p_out2) {
        for (int i = 0; i < p_batch; ++i) {
            for (int j = 0; j < p_n; ++j) {
#pragma HLS PIPELINE
                t_DataType l_word = p_in.read();
                if ((i % 2) == 0) {
                    p_out1.write(l_word);
                } else {
                    p_out2.write(l_word);
                }
            }
        }
    }

    void merge(unsigned int p_n,
               unsigned int p_batch,
               DdrStream& p_in1,
               DdrStream& p_in2,
               DdrStream& p_out,
               unsigned int p_reuseNum) {
        for (int i = 0; i < p_batch; ++i) {
            for (int r = 0; r < p_reuseNum; ++r) {
                for (int j = 0; j < p_n; ++j) {
#pragma HLS PIPELINE
                    t_DataType l_word;
                    if ((i % 2) == 0) {
                        l_word = p_in1.read();
                    } else {
                        l_word = p_in2.read();
                    }
                    p_out.write(l_word);
                }
            }
        }
    }

   private:
    t_BufferType buffer_0, buffer_1;
};
}
}
#endif

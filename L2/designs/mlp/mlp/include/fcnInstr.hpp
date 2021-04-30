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
/**
 *  @file instr.hpp
 *  @brief HPC_CG Level 1 template function implementation.
 *
 */

#ifndef XF_HPC_MLP_ISA_HPP
#define XF_HPC_MLP_ISA_HPP

#include "assert.h"
#include "memInstr.hpp"

namespace xf {
namespace hpc {
namespace mlp {

/** enum class ActFunc_t defines a set of activation function types
 */
enum class ActFunc_t : uint8_t { LINEAR = 1, RELU = 2, SIGMOID = 3, TANSIG = 4 };

/** Class FcnInstr defines a set of parameters used in between host and the kernel
 */
template <unsigned int t_InstrBytes>
class FcnInstr : public MemInstr<t_InstrBytes> {
    typedef MemInstr<t_InstrBytes> super;

   public:
    FcnInstr() { m_Activation = static_cast<uint8_t>(ActFunc_t::LINEAR); }

    void setInVecSize(uint32_t p_InVecSize) { m_InVecSize = p_InVecSize; }
    uint32_t getInVecSize() const { return m_InVecSize; }

    void setOutVecSize(uint32_t p_OutVecSize) { m_OutVecSize = p_OutVecSize; }
    uint32_t getOutVecSize() const { return m_OutVecSize; }

    void setActivation(ActFunc_t p_Activation) { m_Activation = static_cast<uint8_t>(p_Activation); }
    ActFunc_t getActivation() const { return static_cast<ActFunc_t>(m_Activation); }

    void setBatch(uint32_t p_Batch) { m_Batch = p_Batch; }
    uint32_t getBatch() const { return m_Batch; }

    void setInputOffset(uint32_t p_InputOffset) { m_InputOffset = p_InputOffset; }
    uint32_t getInputOffset() const { return m_InputOffset; }

    void setBiasOffset(uint32_t p_BiasOffset) { m_BiasOffset = p_BiasOffset; }
    uint32_t getBiasOffset() const { return m_BiasOffset; }

    void setOutputOffset(uint32_t p_OutputOffset) { m_OutputOffset = p_OutputOffset; }
    uint32_t getOutputOffset() const { return m_OutputOffset; }

    void setWeightsOffset(uint32_t p_WeightsOffset) { m_WeightsOffset = p_WeightsOffset; }
    uint32_t getWeightsOffset() const { return m_WeightsOffset; }

    void setClock(uint64_t p_Clock) { m_Clock = p_Clock; }
    uint64_t getClock() const { return m_Clock; }

    template <typename T>
    void store(T* p_mem) {
        encode();
        super::template store<T>(p_mem);
    }

    template <typename T>
    void load(T* p_mem) {
        super::template load<T>(p_mem);
        decode();
    }

#ifndef __SYNTHESIS__
    friend std::ostream& operator<<(std::ostream& os, FcnInstr& cgInstr) {
        os << "m_InVecSize: " << cgInstr.m_InVecSize << ", ";
        os << "m_OutVecSize: " << cgInstr.m_OutVecSize << ", ";
        os << "m_Batch: " << cgInstr.m_Batch << ", ";
        os << "m_Activation: " << static_cast<uint16_t>(cgInstr.m_Activation) << ", ";
        os << "m_Clock: " << cgInstr.m_Clock << ", ";
        return os;
    }
#endif

   protected:
    void encode() {
#ifdef __SYNTHESIS__
#pragma HLS INLINE
#endif
        unsigned int l_loc = 0;
        super::template encode<uint32_t>(l_loc, m_InVecSize);
        super::template encode<uint32_t>(l_loc, m_OutVecSize);
        super::template encode<uint32_t>(l_loc, m_Batch);
        super::template encode<uint32_t>(l_loc, m_InputOffset);
        super::template encode<uint32_t>(l_loc, m_BiasOffset);
        super::template encode<uint32_t>(l_loc, m_OutputOffset);
        super::template encode<uint32_t>(l_loc, m_WeightsOffset);
        super::template encode<uint8_t>(l_loc, m_Activation);
        super::template encode<uint64_t>(l_loc, m_Clock);
    }
    void decode() {
#ifdef __SYNTHESIS__
#pragma HLS INLINE
#endif
        unsigned int l_loc = 0;
        super::template decode<uint32_t>(l_loc, m_InVecSize);
        super::template decode<uint32_t>(l_loc, m_OutVecSize);
        super::template decode<uint32_t>(l_loc, m_Batch);
        super::template decode<uint32_t>(l_loc, m_InputOffset);
        super::template decode<uint32_t>(l_loc, m_BiasOffset);
        super::template decode<uint32_t>(l_loc, m_OutputOffset);
        super::template decode<uint32_t>(l_loc, m_WeightsOffset);
        super::template decode<uint8_t>(l_loc, m_Activation);
        super::template decode<uint64_t>(l_loc, m_Clock);
    }

   public:
    uint32_t m_InVecSize = 0;
    uint32_t m_OutVecSize = 0;
    uint32_t m_Batch = 0;
    uint32_t m_InputOffset = 0;
    uint32_t m_BiasOffset = 0;
    uint32_t m_OutputOffset = 0;
    uint32_t m_WeightsOffset = 0;
    uint8_t m_Activation;
    uint64_t m_Clock = 0;
};

} // namespace
}
}

#endif

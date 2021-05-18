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
#ifndef HPC_MLP_BASE_HPP__
#define HPC_MLP_BASE_HPP__
#include <cstdint>
#include <vector>
#include <algorithm>
#include <chrono>
#include "fcnInstr.hpp"
#include "mlp.hpp"
#include "fpga.hpp"
#include "binFiles.hpp"
#include "mlpKernel.hpp"

using namespace std;
using namespace xf::hpc::mlp;

namespace xilinx_apps {
namespace mlp {

    struct Options {
        uint32_t numDevices;
        uint8_t deviceIds[HPC_maxNumDevs];
        string xclbinNames[HPC_maxNumDevs];
        uint8_t numCUsOnDevice[HPC_maxNumDevs];
        string cuNames[HPC_maxNumDevs][HPC_maxNumCUsPerDev];
    };

    class MLPBase {
        public:
            MLPBase(const Options& options) {
                for (unsigned int i=0; i<options.numDevices; ++i) {
                    m_devices.push_back(new FPGA(options.deviceIds[i]));
                    m_devices.back()->xclbin(options.xclbinNames[i]);

                    for (unsigned int j=0; j<options.numCUsOnDevice[i]; ++j) {
                        m_cus.push_back(new MLPKernel<HPC_dataType, HPC_instrBytes>(m_devices[i], HPC_numChannels, HPC_vecChannels, HPC_parEntries));
                        m_cus.back()->getCU(options.cuNames[i][j]);
                    }   
                }   
            }
            ~MLPBase() {
            }

            void addEmptyModel(const uint32_t p_numLayers) {
                m_models.push_back(new MLP<HPC_dataType>(p_numLayers));
            }

            void setDim(const uint32_t p_modelId, const void* dims) {
                m_models[p_modelId]->setDim((uint32_t*)(dims));
            }
            
            void setActFunc(const uint32_t p_modelId, const uint32_t p_layerId, const string &p_act) {
                auto l_func = ActFunc_t::NONE;
                if (p_act == "linear") {
                    l_func = ActFunc_t::LINEAR;
                }
                else if (p_act == "relu") {
                    l_func = ActFunc_t::RELU;
                }
                else if (p_act == "sigmoid") {
                    l_func = ActFunc_t::SIGMOID;
                }
                else if (p_act == "tansig") {
                    l_func = ActFunc_t::TANSIG;
                }
                m_models[p_modelId]->setActFunc(p_layerId, l_func);
            }

            void setAllActFunc(const uint32_t p_modelId, const string &p_act) {
                auto l_func = ActFunc_t::NONE;
                if (p_act == "linear") {
                    l_func = ActFunc_t::LINEAR;
                }
                else if (p_act == "relu") {
                    l_func = ActFunc_t::RELU;
                }
                else if (p_act == "sigmoid") {
                    l_func = ActFunc_t::SIGMOID;
                }
                else if (p_act == "tansig") {
                    l_func = ActFunc_t::TANSIG;
                }
                m_models[p_modelId]->setActFunc(l_func);
            }

            void setLayer(const uint32_t p_modelId, const uint32_t p_layerId, void* p_weights, void* p_bias) {
                m_models[p_modelId]->setLayer(p_layerId, (HPC_dataType*)p_weights, (HPC_dataType*)p_bias);
            } 

            void setAllLayers(const uint32_t p_modelId, void** p_weights, void** p_bias) {
                m_models[p_modelId]->setLayer((HPC_dataType**)(p_weights), (HPC_dataType**)(p_bias));
            }
            
            void loadLayersFromFile(const uint32_t p_modelId, const void* p_path) {
                m_models[p_modelId]->loadLayer((char*)(p_path));
            } 

            void loadModel(const uint32_t p_modelId=0, const uint32_t p_cuId=0) {
                m_cus[p_cuId]->loadModel(m_models[p_modelId]);
            }

            double inference(host_buffer_t<HPC_dataType>& p_x, host_buffer_t<HPC_dataType>& p_y, const uint32_t p_modelId=0, const uint32_t p_cuId=0) {
                double sec = m_cus[p_cuId]->inference(p_x, p_y);
                return sec;
            }
            void clear() {
                for (int i=0; i<m_devices.size(); ++i) {
                    delete m_devices[i];       
                }
                for (int i=0; i<m_cus.size(); ++i) {
                    delete m_cus[i];       
                }
                for (int i=0; i<m_models.size(); ++i) {
                    delete m_models[i];
                }
            }
        private:
            vector<FPGA* > m_devices;
            vector<MLPKernel<HPC_dataType, HPC_instrBytes>* > m_cus;
            vector<MLP<HPC_dataType>* > m_models;
    };

}
};
#endif
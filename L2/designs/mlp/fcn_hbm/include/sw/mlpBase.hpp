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
using MaxNumDevs=8;
using MaxNumCUs = 8;

namespace xilinx_apps {
namespace mlp {

    struct Options {
        int32_t numDevices;
        int8_t deviceIds[MaxNumDevs]
        string xclbinNames[MaxNumDevs];
        string numCUsOnDevice[MaxNumDevs];
        string cuNames[MaxNumDevs][MaxNumCUs];
    };

    class MLPBase {
        public:
            MLPBase(const Options& options) {
                for (int i=0; i<options.numDevices; ++i) {
                    m_devices.push_back(new FPGA(options.deviceIds[i]));
                    m_devices.back().xclbin(options.xclbinNames[i]);

                    for (int j=0; j<options.numCUsOnDevice[i]; ++j) {
                        m_cus.push_back(new MLPKernel<HPC_dataType, HPC_instrBytes>(&(m_devices[i]), HPC_numChannels, HPC_vecChannels, HPC_parEntries));
                        m_cus.back().getCU(options.cuNames[i][j]);
                    }   
                }   
            }
            ~MLPBase() {
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

            void addEmptyModel(int numLayers) {
                m_models.push_back(new MLP<HPC_dataType>(numLayers));
            }

            void setDim(int modelId, const uint32_t* dims) {
                m_models[i]->setDim(dims);
            }
            
            void setActFunc(int modelId, int layerId, uint8_t act) {
                m_models[i]->setActFunc(layerId, static_cast<ActFunc_t>(act));
            }

            void setLayer(int modelId, int layerId, HPC_dataType* weights, HPC_dataType* bias) {
                m_models[i]->setLayer(layerId, weights, bias);
            } 

            void inference(host_buffer_t<HPC_dataType>& p_x, host_buffer_t<HPC_dataType>& p_y, int p_modelId=0, p_cuId=0) {
                m_cus[p_cuId].inference(*(m_models[p_modelId]))
            }
        private:
            vector<FPGA* > m_devices;
            vector<MLPKernel<HPC_dataType, HPC_instrBytes>* > m_cus;
            vector<MLP<HPC_dataType>* > m_models;
    }

}
};
#endif

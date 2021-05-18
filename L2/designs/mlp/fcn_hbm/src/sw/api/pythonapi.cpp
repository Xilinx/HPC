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
#include <string>
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "mlpBase.hpp"

using namespace std;
using namespace xilinx_apps::mlp;

namespace py = pybind11;

namespace xilinx_apps {
namespace mlp {

class PyMLPWrapper : public MLPBase {
    PyMLPWrapper(const Options& options) : MLPBase(options) {}

    void addEmptyModel(const uint32_t p_numLayers) { MLPBase::addEmptyModel(p_numLayers); }

    void setDim(const uint32_t p_modelId, const void* dims) { MLPBase::setDim(p_modelId, dims); }

    void setActFunc(const uint32_t p_modelId, const uint32_t p_layerId, const string& p_act) {
        MLPBase::setActFunc(p_modelId, p_layerId, p_act);
    }

    void setAllActFunc(const uint32_t p_modelId, const string& p_act) { MLPBase::setAllActFunc(p_modelId, p_act); }

    void setLayer(const uint32_t p_modelId, const uint32_t p_layerId, void* p_weights, void* p_bias) {
        MLPBase::setLayer(p_modelId, p_layerId, p_weights, p_bias);
    }

    void loadModel(const uint32_t p_modelId = 0, const uint32_t p_cuId = 0) { MLPBase::loadModel(p_modelId, p_cuId); }

    double inference(void* p_x, void* p_y, const uint32_t p_modelId = 0, const uint32_t p_cuId = 0) {
            return (MLPBase::inference(p_x, p_y, p_modelId, p_cuId);
    }
};
}
}

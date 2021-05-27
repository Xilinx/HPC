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
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "mlpBase.hpp"

using namespace std;
using namespace xilinx_apps::mlp;

namespace py = pybind11;

namespace xilinx_apps {
namespace mlp {

class PyMLPWrapper : public MLPBase {
   public:
    PyMLPWrapper(const string& str, const uint32_t num) : MLPBase(str, num) {}
    PyMLPWrapper(const Options& options) : MLPBase(options) {}

    void addEmptyModel(const uint32_t p_numLayers) { MLPBase::addEmptyModel(p_numLayers); }

    void setDim(const uint32_t p_modelId, py::array_t<uint32_t> p_dims) {
        auto l_dimBuf = p_dims.request();
        MLPBase::setDim(p_modelId, l_dimBuf.ptr);
    }

    void setActFunc(const uint32_t p_modelId, const uint32_t p_layerId, const string& p_act) {
        MLPBase::setActFunc(p_modelId, p_layerId, p_act);
    }

    void setAllActFunc(const uint32_t p_modelId, const string& p_act) { MLPBase::setAllActFunc(p_modelId, p_act); }

    void setLayer(const uint32_t p_modelId,
                  const uint32_t p_layerId,
                  py::array_t<HPC_dataType> p_weights,
                  py::array_t<HPC_dataType> p_bias) {
        auto p_weightsBuf = p_weights.request();
        auto p_biasBuf = p_bias.request();
        MLPBase::setLayer(p_modelId, p_layerId, p_weightsBuf.ptr, p_biasBuf.ptr);
    }

    void loadModel(const uint32_t p_modelId = 0, const uint32_t p_cuId = 0) { MLPBase::loadModel(p_modelId, p_cuId); }

    double predict(const uint32_t p_batch,
                   py::array_t<HPC_dataType> p_x,
                   py::array_t<HPC_dataType> p_y,
                   const uint32_t p_cuId = 0) {
        return (
            MLPBase::inference(p_batch, (HPC_dataType*)p_x.request().ptr, (HPC_dataType*)p_y.request().ptr, p_cuId));
    }

    double predictOnCPU(const uint32_t p_batch, py::array_t<HPC_dataType> p_x, py::array_t<HPC_dataType> p_y) {
        return (MLPBase::inferenceOnCPU(p_batch, (HPC_dataType*)p_x.request().ptr, (HPC_dataType*)p_y.request().ptr));
    }

    double predictOnAll(const uint32_t p_batch, py::array_t<HPC_dataType> p_x, py::array_t<HPC_dataType> p_y) {
        return (MLPBase::inferenceOnAll(p_batch, (HPC_dataType*)p_x.request().ptr, (HPC_dataType*)p_y.request().ptr));
    }

    void clear() { MLPBase::clear(); }
};
}
}

PYBIND11_MODULE(xilAlveoMLP, pc) {
    pc.doc() = "Python bindings for the Xilinx Alveo MLP library";

    py::class_<PyMLPWrapper>(pc, "alveomlp")
        .def(py::init<const string, uint32_t>())
        .def("addEmptyModel", &PyMLPWrapper::addEmptyModel, "add an empty MLP model with given number of layers")
        .def("setDim", &PyMLPWrapper::setDim, "set dimensions of each layer")
        .def("setActFunc", &PyMLPWrapper::setActFunc, "set activation function of a given layer in a given model")
        .def("setLayer", &PyMLPWrapper::setLayer, "set the weights and bias of a given layer in a given model")
        .def("loadModel", &PyMLPWrapper::loadModel, "load the given model to the device meory of a given CU")
        .def("predictOnCPU", &PyMLPWrapper::predictOnCPU, "given a batch of input, predict the outputs with the model")
        .def("predictOnAll", &PyMLPWrapper::predictOnAll, "given a batch of input, predict the outputs with the model")
        .def("predict", &PyMLPWrapper::predict, "given a batch of input, predict the outputs with the model")
        .def("clear", &PyMLPWrapper::clear, "clear and release all resources");
}
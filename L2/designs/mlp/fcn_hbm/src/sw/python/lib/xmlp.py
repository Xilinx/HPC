# Copyright 2019 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import math
import numpy as np
from tensorflow import keras
from xilAlveoMLP import alveomlp


class CpuModel:
    def __init__(self):
        self.model = keras.Sequential()
        self.inDim = 0
        self.layers = 0

    def build(self, p_model, p_layerIds, p_dims, p_actFuncs):
        self.inDim = p_dims[0]
        self.layers = len(p_layerIds)
        for i in range(self.layers):
            l_layerId = p_layerIds[i]
            l_layer = p_model.layers[l_layerId]
            self.model.add(l_layer)

    def predict(self, p_xMat):
        l_mat = np.reshape(p_xMat, (-1, self.inDim)).astype(np.float32)
        l_yOut = self.model.predict(l_mat, batch_size=p_xMat.shape[0])
        return l_yOut


class AlveoModel:
    def __init__(
            self,
            alveomlp,
            weightChannels,
            vecChannels,
            parEntries,
            actFuncs):
        self.weightChannels = weightChannels
        self.vecChannels = vecChannels
        self.parEntries = parEntries
        self.devFuncs = actFuncs
        self.numLayers = 0
        self.lastActFunc = ''
        self.inDim = 0
        self.outDim = 0
        self.dims = []
        self.weights = []
        self.bias = []
        self.actFuncs = []
        self.alveomlp = alveomlp

    def build(self, p_model, p_layerIds, p_dims, p_actFuncs, numDev=3):
        self.numLayers = len(p_layerIds)
        self.alveomlp.addEmptyModel(self.numLayers)
        self.inDim = p_dims[0]
        self.outDim = p_dims[-1]
        self.lastActFunc = p_actFuncs[-1]
        self.alveomlp.setDim(0, np.array(p_dims, dtype=np.uint32))
        for i in range(self.numLayers):
            l_layer = p_model.layers[p_layerIds[i]]
            l_weights = l_layer.weights[0][:].numpy()
            l_bias = l_layer.weights[1][:].numpy()
            l_weights = np.transpose(l_weights)
            self.alveomlp.setLayer(0, i, l_weights.flatten(), l_bias)
            if p_actFuncs[i] in self.devFuncs:
                self.alveomlp.setActFunc(0, i, p_actFuncs[i])
            else:
                self.alveomlp.setActFunc(0, i, "linear")
        for i in range(numDev):
            self.alveomlp.loadModel(0, i)

    def predict(self, p_xMat, numDev=3):
        assert p_xMat.size % self.inDim == 0
        l_batch = p_xMat.size // self.inDim
        l_resMat = np.zeros(l_batch * self.outDim, dtype=np.float32)
        l_hwTime = self.alveomlp.predictOnAll(l_batch,
                                              p_xMat, l_resMat)
        print("INFO: hardware inference time: %.2f ms." % (l_hwTime * 1000))
        l_resMat = np.reshape(l_resMat, (l_batch, self.outDim))
        if self.lastActFunc == 'softmax':
            l_exp = np.exp(l_resMat)
            for j in range(l_resMat.shape[0]):
                l_sum = np.sum(l_exp[j])
                l_resMat[j] = l_exp[j] / l_sum
        return l_resMat


class xMLPInf:
    def __init__(self, devConf):
        self.weightChannels = devConf["devices"][0]["weightChannels"]
        self.vecChannels = devConf["devices"][0]["vectorChannels"]
        self.parEntries = devConf["devices"][0]["elements"]
        self.devActFuncs = devConf["devices"][0]["activation_functions"]
        self.models = []

    def checkModel(self, p_model):
        l_isMLP = True
        l_dims = []
        l_actFuncs = []
        l_layerIds = []
        for l_id in range(len(p_model.layers)):
            layer = p_model.layers[l_id]
            l_conf = layer.get_config()
            if 'batch_input_shape' in l_conf.keys():
                continue
            elif isinstance(layer, keras.layers.Dense):
                l_dims.append(layer.weights[0].shape[0])
                l_act = l_conf['activation']
                l_actFuncs.append(l_act)
                l_layerIds.append(l_id)
            else:
                l_isMLP = False
        if l_isMLP:
            l_dims.append(p_model.layers[-1].weights[0].shape[1])
        return [l_isMLP, l_layerIds, l_dims, l_actFuncs]

    def createModels(
            self,
            p_model,
            p_alveomlp,
            p_layerIds,
            p_dims,
            p_actFuncs):
        l_layers = len(p_layerIds)
        l_sId = 0
        l_forCpu = True
        l_resModels = []
        while l_sId < l_layers:
            l_eId = l_layers
            for func in p_actFuncs:
                if l_forCpu and (func in self.devActFuncs):
                    l_eId = p_actFuncs.index(func)
                    break
                elif (not l_forCpu) and (not (func in self.devActFuncs)):
                    l_eId = p_actFuncs.index(func) + 1
                    break

            if l_forCpu and l_eId > l_sId:
                l_cpuModel = CpuModel()
                l_cpuModel.build(p_model,
                                 p_layerIds[l_sId:l_eId],
                                 p_dims[l_sId:l_eId + 1],
                                 p_actFuncs[l_sId:l_eId])
                l_sId = l_eId
                l_resModels.append(l_cpuModel)
            elif (not l_forCpu) and l_eId > l_sId:
                l_alveoModel = AlveoModel(
                    p_alveomlp,
                    self.weightChannels,
                    self.vecChannels,
                    self.parEntries,
                    self.devActFuncs)
                l_alveoModel.build(p_model,
                                   p_layerIds[l_sId:l_eId],
                                   p_dims[l_sId:l_eId + 1],
                                   p_actFuncs[l_sId:l_eId])
                l_sId = l_eId
                l_resModels.append(l_alveoModel)
            l_forCpu = not l_forCpu
        return l_resModels

    def buildModels(self, p_model, p_alveomlp):
        [l_isMlp, l_layerIds, l_dims, l_actFuncs] = self.checkModel(p_model)
        if l_isMlp:
            self.models = self.createModels(
                p_model, p_alveomlp, l_layerIds, l_dims, l_actFuncs)
            return True
        else:
            return False

    def predict(self, p_xMat):
        l_resMat = p_xMat
        for model in self.models:
            l_resMat = model.predict(l_resMat)
        return l_resMat

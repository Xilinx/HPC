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
#include <iostream>
#include "mlp.hpp"
using namespace xf::hpc::mlp;
extern "C" {

void* createModel(int num) {
    return (void*)new MLP<HPC_dataType>(num);
}
void destroyModel(void* model) {
    delete (MLP<HPC_dataType>*)model;
}
void setDim(void* model, const uint32_t* p_dims) {
    ((MLP<HPC_dataType>*)model)->setDim(p_dims);
}
void setActFunc(void* model, uint8_t p_act) {
    ((MLP<HPC_dataType>*)model)->setActFunc(static_cast<ActFunc_t>(p_act));
}
void setActFuncByID(void* model, uint32_t p_id, uint8_t p_act) {
    ((MLP<HPC_dataType>*)model)->setActFunc(p_id, static_cast<ActFunc_t>(p_act));
}
void loadLayer(void* model, const char* path) {
    ((MLP<HPC_dataType>*)model)->loadLayer(path);
}
void setLayer(void* model, HPC_dataType** weights, HPC_dataType** bias) {
    ((MLP<HPC_dataType>*)model)->setLayer(weights, bias);
}
}

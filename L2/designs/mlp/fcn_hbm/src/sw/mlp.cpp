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

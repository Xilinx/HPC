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

from os import path
import subprocess
import argparse
import time
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
from tensorflow.keras.models import load_model

def get_uncompiled_model():
    inputs = keras.Input(shape=(784,), name="digits")
    x = layers.Dense(64, activation="relu", name="dense_1")(inputs)
    x = layers.Dense(64, activation="relu", name="dense_2")(x)
    outputs = layers.Dense(10, activation="softmax", name="predictions")(x)
    model = keras.Model(inputs=inputs, outputs=outputs)
    return model


def get_compiled_model():
    model = get_uncompiled_model()
    model.compile(
        optimizer="rmsprop",
        loss="sparse_categorical_crossentropy",
        metrics=["sparse_categorical_accuracy"],
    )
    return model


def train(p_modelFileName, p_inFileName, p_refFileName, p_modelName):
    print("INFO: Trainning the model {}".format(p_modelName))
    l_kerasFunc = 'keras.datasets.'+ p_modelName + '.load_data()'
    (x_train, y_train), (x_test, y_test) = eval(l_kerasFunc)
    x_test = x_test / 255
    model = get_compiled_model()
    x_train = x_train.reshape(60000, 784).astype(np.float32) / 255
    history = model.fit(
        x_train,
        y_train,
        batch_size=64,
        epochs=5,
        validation_split=0.2)

    model.save(p_modelFileName)
    x_test.astype(np.float32).tofile(p_inFileName)
    y_test.astype(np.float32).tofile(p_refFileName)
    print("INFO: Training done.")


def evaluate(p_modelFileName, p_inFileName, p_outFileName, p_refFileName, p_modelName, p_evaluate):
    print("INFO: Inference from the model {}".format(p_modelName))
    model = load_model(p_modelFileName)
    x_test = np.fromfile(p_inFileName, dtype=np.float32)
    if p_evaluate:
        y_test = np.fromfile(p_refFileName, dtype=np.float32)
    model.summary()
    l_sTime = time.time_ns()
    x_test = x_test.reshape(10000, 784).astype(np.float32)
    y_out = model.predict(x_test, batch_size=x_test.shape[0])
    l_eTime = time.time_ns()
    y_out.astype(np.float32).tofile(p_outFileName)
    if (p_evaluate):
        test_scores = model.evaluate(x_test, y_test, verbose=2)
        print("INFO: scores are {}".format(test_scores))
    l_ms = (l_eTime - l_sTime)/(10**6)
    return l_ms

def verify_mlp(p_model):
    l_isMLP = True
    i=0
    l_inputSize = 1
    for layer in p_model.layers:
        l_conf = layer.get_config()
        if 'batch_input_shape' in l_conf.keys():
            print("INFO: Input layer")
        elif isinstance(layer, keras.layers.Dense):
            print("INFO: Dense layer")
            print(layer.name, layer.weights[0].shape, layer.weights[1].shape)
            print(l_conf['activation'])
            if i==0:
                l_inputSize = layer.weights[0].shape[0]
                i +=1
        else:
            print("ERROR: not a MLP model")
            l_isMLP = False
    return [l_isMLP, l_inputSize]

def matrix_run(p_weights, p_mat, p_bias, p_act, p_batchSize):
    l_resMat = np.matmul(p_mat, p_weights)
    l_resMat += p_bias

    if p_act == 'relu':
        l_resMat = np.maximum(l_resMat,0)
    elif p_act == 'sigmoid':
        l_resMat = 1/(1+np.exp(-l_resMat))
    elif p_act == 'softmax':
        l_exp = np.exp(l_resMat)
        for i in range(l_resMat.shape[0]):
            l_sum = np.sum(l_exp[i])
            l_resMat[i] = l_exp[i]/l_sum
    return l_resMat

def fcn_run(p_weights, p_mat, p_bias, p_act, p_batchSize):
    l_totalBatches = p_mat.shape[0]
    l_runSize = l_totalBatches // p_batchSize
    l_resMat = np.zeros((p_mat.shape[0], p_weights.shape[0]))
    for i in range(p_batchSize):
        l_mat = p_mat[i*l_runSize: (i+1)*l_runSize][:]
        for j in range(l_mat.shape[0]):
            l_resMat[i*l_runSize+j][:] = p_weights.dot(l_mat[j][:])
    assert(p_bias.shape[0] == l_resMat.shape[1])
    for i in range(l_resMat.shape[0]):
        l_resMat[i][:] += p_bias

    if p_act == 'relu':
        l_resMat = np.maximum(l_resMat,0)
    elif p_act == 'sigmoid':
        l_resMat = 1/(1+np.exp(-l_resMat))
    elif p_act == 'softmax':
        l_exp = np.exp(l_resMat)
        for i in range(l_resMat.shape[0]):
            l_sum = np.sum(l_exp[i])
            l_resMat[i] = l_exp[i]/l_sum
    return l_resMat 
        

def fcn_inf(p_modelFileName, p_inFileName, p_fcnOutFileName, p_batchSize):
    l_model = load_model(p_modelFileName)
    [l_isMLP, l_inputSize] = verify_mlp(l_model)
    if l_isMLP:
        l_mat = np.fromfile(p_inFileName, dtype=np.float32)
        l_mat = np.reshape(l_mat, (-1, l_inputSize))
        l_sTime = time.time_ns()
        for i in range(1, len(l_model.layers)):
            l_layer = l_model.layers[i]
            l_conf = l_layer.get_config()
            l_weights = l_layer.weights[0][:].numpy()
            l_bias = l_layer.weights[1][:].numpy()
            l_act = l_conf['activation']
            l_weights = np.transpose(l_weights)
            l_mat = fcn_run(l_weights, l_mat, l_bias, l_act, p_batchSize)
    l_eTime = time.time_ns()
    l_mat.astype(np.float32).tofile(p_fcnOutFileName)
    l_ms = (l_eTime - l_sTime)/(10**6)
    return l_ms
            
def verify_inf(p_fileName, p_goldenFileName):
    l_arr = np.fromfile(p_fileName, dtype=np.float32)
    l_refArr = np.fromfile(p_goldenFileName, dtype=np.float32)
    l_equal = np.allclose(l_arr, l_refArr)
    return l_equal


def process_model(p_needTrain, p_inf, p_evaluate, p_fcn, p_batchSize, p_modelPath, p_modelName):
    l_path = p_modelPath +'/'+p_modelName
    if not path.exists(l_path):
        subprocess.run(["mkdir","-p",l_path])
    l_modelFileName = l_path+'/' + p_modelName +'.h5'
    l_inFileName = l_path+'/inputs.bin'
    l_outFileName = l_path+'/outputs.bin'
    l_refFileName = l_path+'/golden.bin'
    if p_needTrain:
        (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
        train(l_modelFileName, l_inFileName, l_refFileName, p_modelName)
    if p_inf:
        l_ms = evaluate(l_modelFileName, l_inFileName, l_outFileName, l_refFileName, p_modelName, p_evaluate)
        print("INFO: Keras inference takes {}ms".format(l_ms))
    if p_fcn:
        l_fcnOutFileName = l_path+'/outputs_fcn.bin'
        l_ms = fcn_inf(l_modelFileName, l_inFileName, l_fcnOutFileName, p_batchSize)
        print("INFO: FCN inference takes {}ms".format(l_ms))
        l_pass = verify_inf(l_fcnOutFileName, l_outFileName)
        if l_pass:
            print("INFO: fcn inference passes verification!")
        else:
            print("ERROR: there are mismathes between fcn inference results and keras ones.")

def main(args):
    if (args.usage):
        print('Usage example:')
        print('python keras_example.py [--train] [--inf --batch_size 2] [--evaluate] [--fcn] [--model_path ./models]  [--model_name mnist]')
        print('python keras_example.py --train --model_path ./models  --model_name mnist')
        print('python keras_example.py --inf  --model_path ./models  --model_name mnist')
        print('python keras_example.py --inf --evaluate  --model_path ./models  --model_name mnist')
        print('python keras_example.py --fcn --batch_size 2 --model_path ./models  --model_name mnist')
        
    else:
        process_model(args.train, args.inf, args.evaluate, args.fcn, args.batch_size, args.model_path, args.model_name)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='An example usage of Keras MLP APIs')
    parser.add_argument('--usage',action='store_true',help='print usage example')
    parser.add_argument('--train',action='store_true',help='train the model')
    parser.add_argument('--inf',action='store_true',help='inference from the model')
    parser.add_argument('--evaluate',action='store_true',help='run keras evaluation for input data and golden reference data')
    parser.add_argument('--fcn',action='store_true',help='use fcn to do inference from .h5 file')
    parser.add_argument('--batch_size',type=int,default=2,help='batch size for input vectors')
    parser.add_argument('--model_path',type=str,default='./models',help='path for .h5 files that contain models and weights')
    parser.add_argument('--model_name',type=str,default='mnist',help='model name')
    args = parser.parse_args()
    main(args)

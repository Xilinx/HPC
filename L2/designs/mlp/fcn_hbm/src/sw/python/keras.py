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
import numpy as np
import h5py
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import keras

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
    model = keras.models.load_model(p_modelFileName)
    x_test = np.fromfile(p_inFileName, dtype=np.float32)
    if p_evaluate:
        y_test = np.fromfile(p_refFileName, dtype=np.float32)
    model.summary()
    x_test = x_test.reshape(10000, 784).astype(np.float32)
    y_out = model.predict(x_test)
    y_out.astype(np.float32).tofile(p_outFileName)
    if (p_evaluate):
        test_scores = model.evaluate(x_test, y_test, verbose=2)
        print("INFO: scores are {}".format(test_scores))

def fcn_inf(p_modelFileName, p_inFileName, p_fcnOutFileName, p_goldenFileName):
    l_model = load_model(p_modelFileName)
    l_model.summary()
    

def process_model(p_needTrain, p_inf, p_evaluate, p_fcn, p_modelPath, p_modelName):
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
        evaluate(l_modelFileName, l_inFileName, l_outFileName, l_refFileName, p_modelName, p_evaluate)
    if p_fcn:
        l_fcnOutFileName = l_path+'/outputs_fcn.bin'
        fcn_inf(l_modelFileName, l_inFileName, l_fcnOutFileName, l_outFileName)

def main(args):
    if (args.usage):
        print('Usage example:')
        print('python keras.py [--train] [--inf] [--evaluate] [--fcn] [--model_path ./models]  [--model_name mnist]')
        print('python keras.py --train --model_path ./models  --model_name mnist')
        print('python keras.py --inf  --model_path ./models  --model_name mnist')
        print('python keras.py --inf --evaluate  --model_path ./models  --model_name mnist')
        print('python keras.py --fcn  --model_path ./models  --model_name mnist')
        
    else:
        process_model(args.train, args.inf, args.evaluate, args.fcn, args.model_path, args.model_name)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='An example usage of Keras MLP APIs')
    parser.add_argument('--usage',action='store_true',help='print usage example')
    parser.add_argument('--train',action='store_true',help='train the model')
    parser.add_argument('--inf',action='store_true',help='inference from the model')
    parser.add_argument('--evaluate',action='store_true',help='run keras evaluation for input data and golden reference data')
    parser.add_argument('--fcn',action='store_true',help='use fcn to do inference from .h5 file')
    parser.add_argument('--model_path',type=str,default='./models',help='path for .h5 files that contain models and weights')
    parser.add_argument('--model_name',type=str,default='mnist',help='model name')
    args = parser.parse_args()
    main(args)

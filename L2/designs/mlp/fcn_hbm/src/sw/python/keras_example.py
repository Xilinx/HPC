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

import os
import subprocess
import argparse
import time
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
from tensorflow.keras.models import load_model
from lib.xmlp import xMLPInf
from xilAlveoMLP import alveomlp
import json


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
    l_kerasFunc = 'keras.datasets.' + p_modelName + '.load_data()'
    (x_train, y_train), (x_test, y_test) = eval(l_kerasFunc)
    x_test = x_test / 255
    model = get_compiled_model()
    x_train = x_train.reshape(-1, 784).astype(np.float32) / 255
    history = model.fit(
        x_train,
        y_train,
        batch_size=64,
        epochs=5,
        validation_split=0.2)

    model.save(p_modelFileName)
    if True:
        np.append(x_train, x_test).astype(np.float32).tofile(p_inFileName)
        np.append(y_train, y_test).astype(np.float32).tofile(p_refFileName)
    else:
        x_test.astype(np.float32).tofile(p_inFileName)
        y_test.astype(np.float32).tofile(p_refFileName)
    print("INFO: Training done.")


def evaluate(
        p_modelFileName,
        p_inFileName,
        p_outFileName,
        p_refFileName,
        p_modelName,
        p_evaluate):
    print("INFO: Inference from the model {}".format(p_modelName))
    model = load_model(p_modelFileName)
    l_inputSize = 0
    for layer in model.layers:
        if isinstance(layer, keras.layers.Dense):
            l_inputSize = layer.weights[0].shape[0]
            break

    x_test = np.fromfile(p_inFileName, dtype=np.float32)
    if p_evaluate:
        y_test = np.fromfile(p_refFileName, dtype=np.float32)
    model.summary()
    l_sTime = time.time_ns()
    x_test = np.reshape(x_test, (-1, l_inputSize)).astype(np.float32)
    y_out = model.predict(x_test, batch_size=x_test.shape[0])
    l_eTime = time.time_ns()
    y_out.astype(np.float32).tofile(p_outFileName)
    if (p_evaluate):
        test_scores = model.evaluate(x_test, y_test, verbose=2)
        print("INFO: scores are {}".format(test_scores))
    l_ms = (l_eTime - l_sTime) / (10**6)
    return l_ms


def xmlp_inf(numDev,
             deviceConfig,
             p_modelFileName,
             p_inFileName,
             p_alveomlp,
             p_xmlpOutFileName):
    l_model = load_model(p_modelFileName)
    l_mlpInf = xMLPInf(deviceConfig, numDev)
    l_isMLP = l_mlpInf.buildModels(l_model, p_alveomlp)
    if l_isMLP:
        l_mat = np.fromfile(p_inFileName, dtype=np.float32)
        l_sTime = time.time_ns()
        l_mat = l_mlpInf.predict(l_mat)
    l_eTime = time.time_ns()
    l_mat.astype(np.float32).tofile(p_xmlpOutFileName)
    l_ms = (l_eTime - l_sTime) / (10**6)
    return l_ms


def verify_inf(p_fileName, p_goldenFileName):
    l_arr = np.fromfile(p_fileName, dtype=np.float32)
    l_refArr = np.fromfile(p_goldenFileName, dtype=np.float32)
    l_equal = l_arr.shape == l_refArr.shape
    compare = np.isclose(l_arr, l_refArr)
    if sum(compare) != compare.size:
        l_equal = False
        print("There are %d mismatches." % (compare.size - sum(compare)))
        index = np.where(compare == False)[:10]
        print("index of mismatches", index)
        print("value from inf", l_arr[index])
        print("value from ref", l_refArr[index])
    return l_equal


def process_model(
        p_needTrain,
        p_inf,
        p_evaluate,
        p_xMLP,
        p_deviceConfig,
        p_devNum,
        p_modelPath,
        p_modelName):
    l_path = p_modelPath + '/' + p_modelName
    if not os.path.exists(l_path):
        subprocess.run(["mkdir", "-p", l_path])
    l_modelFileName = l_path + '/model.h5'
    l_inFileName = l_path + '/inputs.bin'
    l_outFileName = l_path + '/golden.bin'
    l_refFileName = l_path + '/evOut.bin'
    if p_needTrain:
        (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
        train(l_modelFileName, l_inFileName, l_refFileName, p_modelName)
    if p_inf:
        l_ms = evaluate(
            l_modelFileName,
            l_inFileName,
            l_outFileName,
            l_refFileName,
            p_modelName,
            p_evaluate)
        print("INFO: Keras inference takes {}ms".format(l_ms))
    if p_xMLP:
        l_xmlpOutFileName = l_path + '/outputs_xmlp.bin'
        with open(p_deviceConfig, 'r') as f:
            jstr = f.read()
        deviceConfig = json.loads(jstr)
        l_alveomlp = alveomlp(jstr, p_devNum)
        l_ms = xmlp_inf(p_devNum,
                        deviceConfig,
                        l_modelFileName,
                        l_inFileName,
                        l_alveomlp,
                        l_xmlpOutFileName)
        print("INFO: xMLP inference takes {} ms".format(l_ms))
        l_pass = verify_inf(l_xmlpOutFileName, l_outFileName)
        if l_pass:
            print("INFO: xMLP inference passes verification!")
        else:
            print(
                "ERROR: there are mismathes between xMLP inference results and keras ones.")


def main(args):
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
    if (args.usage):
        print('Usage example:')
        print(
            'python keras_example.py [--train] [--kinf] [--evaluate] [--xinf] [--model_path ./models]  [--model_name mnist]')
        print('python keras_example.py --train --model_path ./models  --model_name mnist')
        print('python keras_example.py --kinf  --model_path ./models  --model_name mnist')
        print('python keras_example.py --kinf --evaluate  --model_path ./models  --model_name mnist')
        print('python keras_example.py --xinf --model_path ./models  --model_name mnist')

    else:
        process_model(
            args.train,
            args.kinf,
            args.evaluate,
            args.xinf,
            args.device_config,
            args.dev_num,
            args.model_path,
            args.model_name)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='An example usage of Keras MLP APIs')
    parser.add_argument(
        '--usage',
        action='store_true',
        help='print usage example')
    parser.add_argument(
        '--train',
        action='store_true',
        help='train the model with keras')
    parser.add_argument(
        '--kinf',
        action='store_true',
        help='keras inference from the model')
    parser.add_argument(
        '--evaluate',
        action='store_true',
        help='run keras evaluation for input data and golden reference data')
    parser.add_argument(
        '--xinf',
        action='store_true',
        help='use xMLP to do inference from .h5 file')
    parser.add_argument(
        '--device_config',
        type=str,
        help='path for json files that contain device configuration')
    parser.add_argument(
        '--lib_path',
        type=str,
        default='.',
        help='path to directory of xilAlveoMLP.so')
    parser.add_argument(
        '--model_path',
        type=str,
        default='./models',
        help='path for .h5 files that contain models and weights')
    parser.add_argument(
        '--dev_num',
        type=int,
        default=1,
        help='model name')
    parser.add_argument(
        '--model_name',
        type=str,
        default='mnist',
        help='model name')
    args = parser.parse_args()
    main(args)

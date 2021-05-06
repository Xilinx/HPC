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

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers


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


def train(filepath, x_train, y_train):

    model = get_compiled_model()
    x_train = x_train.reshape(60000, 784).astype("float32") / 255
    history = model.fit(
        x_train,
        y_train,
        batch_size=64,
        epochs=5,
        validation_split=0.2)

    model.save(filepath)


def evaluate(filepath, x_test, y_test):
    model = keras.models.load_model(filepath)
    model.summary()
    x_test = x_test.reshape(10000, 784).astype("float32") / 255
    test_scores = model.evaluate(x_test, y_test, verbose=2)


if __name__ == "__main__":
    (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
    filepath = "./models.h5"
    train(filepath, x_train, y_train)
    evaluate(filepath, x_test, y_test)

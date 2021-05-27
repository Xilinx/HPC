#!/usr/bin/env python3
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

import numpy as np
import time
import sys
import os
import argparse
import pdb
from genFcn import *


class MLP:
    def __init__(self, layers, act, dtype):
        self.n = len(layers) - 1
        self.layers = list()
        for i in range(self.n):
            self.layers.append(FCN(layers[i], layers[i + 1], act, dtype))
        self.layers[-1].act = None

    def __call__(self, x):
        for fcn in self.layers:
            x = fcn(x)
        return x

    def tofile(self, path):
        for i in range(self.n):
            self.layers[i].tofile(path, i)


def main(args):
    if args.datatype == 'float':
        dtype = np.float32
    elif args.datatype == 'double':
        dtype = np.float64
    else:
        sys.exit("Wrong data type received.")

    mlp = MLP(args.layers, eval(args.act), dtype)
    mlp.tofile(args.path)

    x = np.random.random(
        (args.batch, args.layers[0])).astype(
        dtype=dtype) * 2 - 1
    x.tofile(os.path.join(args.path, 'in.mat'))

    start = time.time_ns()

    y = mlp(x)

    stop = time.time_ns()
    print("Python execution time: %f seconds." % ((stop - start) * 1e-9))

    y.tofile(os.path.join(args.path, 'out.mat'))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Generate MLP.')
    parser.add_argument(
        '--path',
        type=str,
        required=True,
        help='result file')
    parser.add_argument(
        '--act',
        type=str,
        default='sigmoid',
        help='Activation function')
    parser.add_argument(
        '--batch',
        type=int,
        default=2,
        help='Input batch')
    parser.add_argument(
        '--layers',
        type=int,
        nargs='+',
        help='layer sizes')
    parser.add_argument(
        '--datatype',
        type=str,
        default='double',
        help="data type"
    )
    args = parser.parse_args()
    main(args)

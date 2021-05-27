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


def linear(x, a=1, b=0):
    return a * x + b


def relu(x):
    x[x < 0] = 0
    return x


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


class FCN:
    def __init__(self, inS, outS, act=None, dtype=np.float32):
        self.inS = inS
        self.outS = outS
        self.act = act
        self.dtype = dtype
        self.W = 2 * np.random.random((outS, inS)
                                      ).astype(dtype=dtype) - 1
        self.b = np.random.random(outS).astype(dtype=dtype) * 2 - 1

    def __call__(self, x):
        y = x @ self.W.transpose() + self.b
        if self.act is not None:
            y = self.act(y)
        return y

    def tofile(self, path, name='0'):
        self.W.tofile(os.path.join(path, 'W_%s.mat' % name))
        self.b.tofile(os.path.join(path, 'b_%s.mat' % name))


def main(args):
    if args.datatype == 'float':
        dtype = np.float32
    elif args.datatype == 'double':
        dtype = np.float64
    else:
        sys.exit("Wrong data type received.")

    fcn = FCN(args.inSize, args.outSize, eval(args.act), dtype)
    x = np.random.random((args.batch, args.inSize)).astype(dtype=dtype) * 2 - 1
    start = time.time_ns()
    y = fcn(x)
    stop = time.time_ns()
    print("Python execution time: %f seconds." % ((stop - start) * 1e-9))

    fcn.tofile(args.path)
    x.tofile(os.path.join(args.path, 'in.mat'))
    y.tofile(os.path.join(args.path, 'out.mat'))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Generate FCN.')
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
        '--outSize',
        type=int,
        default=32,
        help='Output vector dim')
    parser.add_argument(
        '--inSize',
        type=int,
        default=32,
        help='Input vector dim')
    parser.add_argument(
        '--datatype',
        type=str,
        default='double',
        help="data type"
    )
    args = parser.parse_args()
    main(args)

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


def main(args):
    if args.datatype == 'float':
        dtype = np.float32
    elif args.datatype == 'double':
        dtype = np.float64
    else:
        sys.exit("Wrong data type received.")

    W = 2 * np.random.random((args.outSize, args.inSize)
                             ).astype(dtype=dtype) - 1
    x = np.random.random((args.batch, args.inSize)).astype(dtype=dtype) * 2 - 1
    b = np.random.random(args.outSize).astype(dtype=dtype) * 2 - 1
    start = time.time_ns()

    y = x @ W.transpose() + b
    y = eval(args.act)(y)

    stop = time.time_ns()
    print("Python execution time: %f seconds." % ((stop - start) * 1e-9))

    W.tofile(os.path.join(args.path, 'W.mat'))
    x.tofile(os.path.join(args.path, 'in.mat'))
    b.tofile(os.path.join(args.path, 'bias.mat'))
    y.tofile(os.path.join(args.path, 'out.mat'))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Generate SPD Matrix.')
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

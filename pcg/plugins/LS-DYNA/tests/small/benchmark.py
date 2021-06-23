#!/usr/bin/env python3
import subprocess
import argparse
import shlex
import os
import pdb
from multiprocessing import Pool
import pandas as pd


def build(target):
    cmdLine = "make -j 8 TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def run(target, model):
    cmdLine = "make run TARGET=%s MODEL_NAME=%s" % (target, model)
    args = shlex.split(cmdLine)
#    subprocess.run(args)


def report(targets):
    t = 'fpga'
    filename = os.path.join("build_output.%s" % t, "benchmark.csv")
    df = pd.read_csv(filename)
    df.sort_values(by=['Dim'], inplace=True, ignore_index=True)
    df = df.rename(columns={'API GFLOPS': '%s API GFLOPS' % t,
                            'HW GFLOPS': '%s HW GFLOPS' % t,
                            'HW Time [ms]': '%s HW Time [ms]' % t,
                            'API Time [ms]': '%s API Time [ms]' % t})
    for t in targets:
        if t == 'fpga':
            continue
        filename = os.path.join("build_output.%s" % t, "benchmark.csv")
        df_t = pd.read_csv(filename)
        df_t.sort_values(by=['Dim'], inplace=True, ignore_index=True)
        df['%s API Time [ms]' % t] = df_t['API Time [ms]']
        df['%s API GFLOPS' % t] = df_t['API GFLOPS']

    df['Perf. fpga_API/csc_API'] = round(df['fpga API GFLOPS'] /
                                         df['csc API GFLOPS'], 5)
    df['Perf. fpga_HW/csc_API'] = round(df['fpga HW GFLOPS'] /
                                        df['csc API GFLOPS'], 5)

    df['Time csc_API/fpga_API'] = round(df['csc API Time [ms]'] /
                                        df['fpga API Time [ms]'], 5)
    df['Time csc_API/fpga_HW'] = round(df['csc API Time [ms]'] /
                                       df['fpga HW Time [ms]'], 5)
    df.to_csv("benchmark.csv", index=False)


def main(model_path):
    targets = ['fpga', 'csc', 'coo']

    if not os.path.isdir(model_path):
        return
    model_names = os.listdir(model_path)

    with Pool(8) as p:
        p.map(build, targets)

    for m in model_names:
        for t in targets:
            run(t, m)
    report(targets)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("benchmark multiple targets and models")
    parser.add_argument('--model_path', type=str, help="path to model files")
    args = parser.parse_args()
    if args.model_path is None:
        print(parser.print_help())
    else:
        main(args.model_path)

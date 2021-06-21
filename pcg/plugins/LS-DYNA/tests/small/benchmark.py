#!/usr/bin/env python3
import subprocess
import shlex
import os
from multiprocessing import Pool
import pandas as pd


def run(target):
    cmdLine = "make run TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def report(targets):
    t = targets[0]
    filename = os.path.join("build_output.%s" % t, "benchmark.csv")
    df = pd.read_csv(filename)
    df = df.rename(columns={'GFLOPS': '%s GFLOPS' % t,
                            'Time [ms]': '%s Time [ms]' % t})
    for t in targets[1:]:
        filename = os.path.join("build_output.%s" % t, "benchmark.csv")
        df_t = pd.read_csv(filename)
        df['%s Time [ms]' % t] = df_t['Time [ms]']
        df['%s GFLOPS' % t] = df['MFLOPs'] / df_t['Time [ms]']

    df['fpga vs csc'] = df['fpga GFLOPS'] / df['csc GFLOPS']

    df.to_csv("benchmark.csv", index=False)


def main():
    targets = ['csc', 'coo', 'fpga']

    with Pool(3) as p:
        p.map(run, targets)
    report(targets)


if __name__ == "__main__":
    main()

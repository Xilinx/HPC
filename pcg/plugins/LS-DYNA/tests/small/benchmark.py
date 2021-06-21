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
                            'HW Time [ms]': '%s HW Time [ms]' % t,
                            'API Time [ms]': '%s API Time [ms]' % t})
    df['fpga HW GFLOPS'] = round(df['MFLOP'] / df['fpga HW Time [ms]'], 5)
    for t in targets[1:]:
        filename = os.path.join("build_output.%s" % t, "benchmark.csv")
        df_t = pd.read_csv(filename)
        df['%s API Time [ms]' % t] = df_t['API Time [ms]']
        df['%s GFLOPS' % t] = round(df['MFLOP'] / df_t['API Time [ms]'], 5)

    df['fpga_API/csc_API'] = round(df['fpga GFLOPS'] / df['csc GFLOPS'], 5)
    df['fpga_HW/csc_API'] = round(df['fpga HW GFLOPS'] / df['csc GFLOPS'], 5)

    df.to_csv("benchmark.csv", index=False)


def main():
    targets = ['fpga', 'csc', 'coo']

    with Pool(3) as p:
        p.map(run, targets)
    report(targets)


if __name__ == "__main__":
    main()

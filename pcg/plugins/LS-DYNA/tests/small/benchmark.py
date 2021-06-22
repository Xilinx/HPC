#!/usr/bin/env python3
import subprocess
import shlex
import os
from multiprocessing import Pool
import pandas as pd


def build(target):
    cmdLine = "make -j 8 TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def run(target):
    cmdLine = "make run TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def report(targets):
    t = 'fpga'
    filename = os.path.join("build_output.%s" % t, "benchmark.csv")
    df = pd.read_csv(filename)
    df = df.rename(columns={'API GFLOPS': '%s API GFLOPS' % t,
                            'HW GFLOPS': '%s HW GFLOPS' % t,
                            'HW Time [ms]': '%s HW Time [ms]' % t,
                            'API Time [ms]': '%s API Time [ms]' % t})
    for t in targets:
        if t == 'fpga':
            continue
        filename = os.path.join("build_output.%s" % t, "benchmark.csv")
        df_t = pd.read_csv(filename)
        df['%s API Time [ms]' % t] = df_t['API Time [ms]']
        df['%s API GFLOPS' % t] = df_t['API GFLOPS']

    df['fpga_API/csc_API'] = round(df['fpga API GFLOPS'] /
                                   df['csc API GFLOPS'], 5)
    df['fpga_HW/csc_API'] = round(df['fpga HW GFLOPS'] /
                                  df['csc API GFLOPS'], 5)

    df.to_csv("benchmark.csv", index=False)


def main():
    targets = ['fpga', 'csc', 'coo']

    with Pool(8) as p:
        p.map(build, targets)
    for t in targets:
        run(t)
    report(targets)


if __name__ == "__main__":
    main()

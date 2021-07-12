#!/usr/bin/env python3
import subprocess
import argparse
import shlex
import os
import sys
import re
import pdb
from multiprocessing import Pool
import pandas as pd


def build(target):
    cmdLine = "make -j 8 TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def run(target, model):
    cmdLine = "make run TARGET=%s MODEL_PATH=%s" % (target, model)
    args = shlex.split(cmdLine)
    subprocess.run(args)


def report(tables, model_name, model_tables, targets):
    for tn in tables.keys():
        tab = pd.concat([model_tables[t][tn] for t in targets], axis=1,
                        ignore_index=True)
        tab.columns = targets
        tab.insert(0, 'model_name', model_name)
        tables[tn] = tables[tn].append(tab)


def parse(target, table_names):
    tables = {n: pd.DataFrame(columns=[target])
              for n in table_names}

    filename = os.path.join("build_output.%s" % target, "mes0000")
    if not os.path.exists(filename):
        sys.exit(1)

    wct_pattern = 'WCT & GFlop'
    total_pattern = '  T o t a l s'
    iter_pattern = 'userLE Jacobi PCG iteration'
    res_pattern = 'two norm of the residual'
    number = r'[+\-\.\d]+E?[+\-\d]*'

    with open(filename, 'r') as fr:
        lines = fr.readlines()
        for line in lines:
            res = re.findall(number, line)
            if line.startswith(total_pattern):
                tables['Totals'] = tables['Totals'].append({target: res[0]},
                                                           ignore_index=True)
                break
            if line.startswith(wct_pattern):
                tables['WCT'] = tables['WCT'].append({target: res[0]},
                                                     ignore_index=True)
                tables['GFLOPS'] = tables['GFLOPS'].append({target: res[1]},
                                                           ignore_index=True)
            if line.startswith(iter_pattern):
                tables['Iteration'] = tables['Iteration'].append({target: res[0]},
                                                                 ignore_index=True)
            if line.startswith(res_pattern):
                tables['Residual'] = tables['Residual'].append({target: res[0]},
                                                               ignore_index=True)
    os.remove(filename)
    return tables


def main(model_path, log_path):
    targets = ['fpga', 'cpu']
    if not os.path.isdir(model_path):
        sys.exit(1)
    model_names = os.listdir(model_path)

    with Pool(8) as p:
        p.map(build, targets)
        pass

    table_names = ['Totals', 'WCT', 'GFLOPS', 'Iteration', 'Residual']
    tables = {n: pd.DataFrame(
        columns=['model_name'] + targets) for n in table_names}

    for m in model_names:
        model_tables = {}
        for t in targets:
            run(t, os.path.join(model_path, m, "%s.k" % m))
            model_tables[t] = parse(t, table_names)
        report(tables, m, model_tables, targets)

    with pd.ExcelWriter(log_path) as writer:
        for tn, tab in tables.items():
            tab.to_excel(writer, sheet_name=tn, index=False)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("benchmark multiple targets and models")
    parser.add_argument('--model_path', required=True,
                        type=str, help="path tlog files")
    parser.add_argument('--log_path', type=str, default='log.xlsx',
                        help="path to model files")
    args = parser.parse_args()
    if args.model_path is None:
        print(parser.print_help())
    else:
        main(args.model_path, args.log_path)

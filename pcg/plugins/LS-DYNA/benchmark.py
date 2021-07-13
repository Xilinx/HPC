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
import numpy as np


def build(target):
    cmdLine = "make -j 8 TARGET=%s" % target
    args = shlex.split(cmdLine)
    subprocess.run(args)


def run(target, model):
    cmdLine = "make run TARGET=%s MODEL_PATH=%s" % (target, model)
    args = shlex.split(cmdLine)
    subprocess.run(args)


def report(tables, model_name, model_tables, targets):
    column_names = ['model_name',  'Calls', 'dim', 'nnz', 'fpga',
                    'cpu']
    for tn in tables.keys():
        tab = pd.concat([model_tables[t][tn] for t in targets], axis=1)
        tab.insert(0, 'model_name', model_name)
        if tn != 'Totals':
            tab['Calls'] = np.arange(1, tab.shape[0] + 1)
            tab = tab.loc[:, ~tab.columns.duplicated()]
            tab = tab.reindex(columns=column_names)
        if tn == 'Totals' or tn == 'WCT':
            tab['speed_up'] = tab['cpu'].astype(
                float) / tab['fpga'].astype(float)
        tables[tn] = tables[tn].append(tab)


def parse(target, table_names):
    tables = {n: pd.DataFrame() for n in table_names}

    filename = os.path.join("build_output.%s" % target, "mes0000")
    if not os.path.exists(filename):
        sys.exit(1)

    wct_pattern = 'WCT'
    dim_pattern = 'userLE Jacobi PCG dim'
    total_pattern = 'T o t a l s'
    iter_pattern = 'userLE Jacobi PCG iteration'
    res_pattern = 'two norm of the residual'
    number = r'[+\-\.\d]+E?[+\-\d]*'

    with open(filename, 'r') as fr:
        lines = fr.readlines()
        dim = 0
        nnz = 0
        for line in lines:
            line = line.strip()
            res = re.findall(number, line)
            if line.startswith(dim_pattern):
                dim = res[0]
                nnz = res[1]

            if line.startswith(total_pattern):
                tables['Totals'] = tables['Totals'].append({target: res[0]},
                                                           ignore_index=True)
                break
            if line.startswith(wct_pattern):
                tables['WCT'] = tables['WCT'].append({'dim': dim,
                                                      'nnz': nnz,
                                                      target: res[0]},
                                                     ignore_index=True)
            if line.startswith(iter_pattern):
                tables['Iteration'] = tables['Iteration'].append({'dim': dim,
                                                                  'nnz': nnz,
                                                                  target: res[0]},
                                                                 ignore_index=True)
            if line.startswith(res_pattern):
                tables['Residual'] = tables['Residual'].append({'dim': dim,
                                                                'nnz': nnz,
                                                                target: res[0]},
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

    table_names = ['Totals', 'WCT', 'Iteration', 'Residual']
    tables = {n: pd.DataFrame() for n in table_names}

    for m in model_names:
        model_tables = {}
        for t in targets:
            tmp_path = os.path.join("build_output.%s" % t, 'tmp')
            if not os.path.exists(tmp_path):
                os.makedirs(tmp_path)
            run(t, os.path.join(model_path, m, "%s.k" % m))
            table_dict = parse(t, table_names)
            for k, v in table_dict.items():
                v.to_csv(os.path.join(tmp_path, "log_%s_%s_%s" % (m, t, k)),
                         index=False)
            model_tables[t] = table_dict
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

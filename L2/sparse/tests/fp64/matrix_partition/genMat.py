# Copyright 2019-2021 Xilinx, Inc.
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
import sys
import os
import subprocess
import argparse
import scipy.io as sio
import scipy.sparse as sp


def main(args):
    if args.datatype == 'float':
        dtype = np.float32
    elif args.datatype == 'double':
        dtype = np.float64
    else:
        sys.exit("Wrong data type received.")


    download_list = open(args.mtx_list, 'r')
    download_names = download_list.readlines()
    for line in download_names:
      
        mtxHttp = line.strip()
        strList = mtxHttp.split('/')
        mtxFileName = strList[len(strList)-1]
        strList = mtxFileName.split('.')
        mtxName = strList[0]
        mtxFullName = './mtx_files/' + mtxName+'/'+ mtxName + '.mtx'
        mtxSigPath = args.sig_path +'/'+mtxName
        if not os.path.exists(mtxSigPath):
            subprocess.run(["mkdir", "-p", mtxSigPath])
        if not os.path.exists(mtxFullName):
            subprocess.run(["wget", mtxHttp, "-P", "./mtx_files"])
            subprocess.run(["tar", "-xzf", './mtx_files/'+mtxFileName, "-C", "./mtx_files"])        

        sparse_mat = sio.mmread(mtxFullName)
    
        row = sparse_mat.row.astype(np.uint32)
        col = sparse_mat.col.astype(np.uint32)
        data = sparse_mat.data.astype(dtype)

        subprocess.run(["mkdir", "-p", args.sig_path+'/'+mtxName+'/'])
        row.tofile(os.path.join(args.sig_path+'/'+mtxName+'/', 'row.bin'))
        col.tofile(os.path.join(args.sig_path+'/'+mtxName+'/', 'col.bin'))
        data.tofile(os.path.join(args.sig_path+'/'+mtxName+'/', 'data.bin'))


        with open(args.sig_path +'/'+mtxName+'/' + "infos.txt", 'w') as info_file:
            info_file.write(mtxName + '\n')
            info_file.write(str(sparse_mat.shape[0]) + '\n')
            info_file.write(str(sparse_mat.shape[1]) + '\n')
            info_file.write(str(sparse_mat.nnz) + '\n')
            info_file.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Read Sparse Matrix.')
    parser.add_argument('--sig_path',type=str,default='./sig_dat',help='directory for storing partition results, default value ./sig_dat')
    parser.add_argument('--mtx_list',type=str,help='a file containing URLs for downloading sprase matrices')
    parser.add_argument(
        '--datatype',
        type=str,
        default='double',
        help="data type"
    )
    args = parser.parse_args()
    main(args)

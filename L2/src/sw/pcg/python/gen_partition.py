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
from matrix_params import sparse_matrix 
from matrix_params import row_block_param
from matrix_params import par_param 
from matrix_params import row_block

class gen_partition:
    def __init__(self, parEntries, accLatency, channels, maxRows, maxCols, memBits):
        self.parEntries,self.accLatency,self.channels = parEntries,accLatency,channels
        self.maxRows,self.maxCols,self.memBits=maxRows,maxCols,memBits
    
    def gen_rbs(p_spm, p_rbParam, p_rb):
        p_rbParam.totalRows=p_spm.m
        p_rbParam.totalRbs = 0
        l_numPars = 0
        if p_spm.sort('r'):
            while len(p_spm.row) != 0:
                l_minRowId = min(p_spm.row)
                l_minColId = (min(p_spm.col) // self.parEntries)*self.parEntries
                l_idx = 0
                while l_idx < len(p_spm.row) and p_spm.row[l_idx] < l_minRowId + self.maxRows:
                    l_idx++
                p_rb.row.append(p_spm.row[0:l_idx])
                p_rb.col.append(p_spm.col[0:l_idx])
                p_rb.data.append(p_spm.data[0:l_idx])
                del p_spm.row[:l_idx]
                del p_spm.col[:l_idx]
                del p_spm.data[:l_idx]
                l_maxColId = max(p_rb.col[p_rbParam.totalRbs]) 
                l_rows = len(set(p_rb.row[p_rbParam.totalRbs])
                l_nnzs = len(p_rb.row[p_rbParam.totalRbs])
                p_rbParam.add_rbIdxInfo(l_minRowId, l_minColId, l_maxColId, l_numPars)
                p_rbParam.add_rbSizeInfo(l_rows, l_nnzs)
                for i in range(6):
                    p_rbParam.add_dummyInfo()
                p_rbParam.totalRbs++
        else:
            print("ERROR: cannot sort matrix in along rows")
            return

    def gen_pars(p_rbParam, p_rb, p_parParam, p_par):
        l_totalRbs = p_rbParam.totalRbs
        p_parParam.totalPars = 0
        for i in range(l_totalRbs):
            l_rbPars = 0
            l_row=[]
            l_col=[]
            l_data=[]
            l_minColId = p_rbParam.get_rbInfo(i, 0)[1] 
            l_nnzs  = p_rbParam.get_rbInfo(i, 1)[1]
            l_sId,l_eId = 0,0
            while (l_nnzs > 0):
                if (l_sId >= l_nnzs):
                    l_minColId = (min(p_rb.col[i]) // self.parEntries) * self.parEntries
                    p_par.row.append(l_row)
                    p_par.col.append(l_col)
                    p_par.data.append(l_data)
                    l_rbPars++
                    l_row,l_col,l_data = [],[],[]
                    l_sId, l_eId = 0,0
                else:
                    while l_eId < l_nnzs and p_rb.col[i][l_eId] < l_minColId + maxCols:
                        l_eId++
                    l_row.extend(p_rb.row[i][l_sId:l_eId])
                    l_col.extend(p_rb.col[i][l_sId:l_eId])
                    l_data.extend(p_rb.data[i][l_sId:l_eId])
                    del p_rb.row[i][l_sId:l_eId]
                    del p_rb.col[i][l_sId:l_eId]
                    del p_rb.data[i][l_sId:l_eId]
                    l_nnzs = l_nnzs - (l_eId - l_sId)
                    while l_sId < l_nnzs and p_rb.col[i][l_sId] >= l_minColId + maxCols
                        l_sId++
                    l_eId = l_sId
            p_rbParam.set_numPars(i, l_rbPars)
            p_parParam.totalPars = p_parParam.totalPars + l_rbPars

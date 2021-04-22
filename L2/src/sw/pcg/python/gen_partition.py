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
from collections import Counter
from matrix_params import * 

class gen_partition:
    def __init__(self, parEntries, accLatency, channels, maxRows, maxCols, memBits):
        self.parEntries,self.accLatency,self.channels = parEntries,accLatency,channels
        self.maxRows,self.maxCols,self.memBits=maxRows,maxCols,memBits
        self.rbParam = row_block_param(memBits, channels)
        self.parParam = par_param(memBits, channels)
        self.nnzStore = nnz_store(memBits, parEntries, accLatency,channels)
    
    def gen_rbs(self, p_spm, p_rb):
        self.rbParam.totalRows=p_spm.m
        self.rbParam.totalRbs = 0
        l_numPars = 0
        if p_spm.sort('r'):
            p_spm.to_list()
            while len(p_spm.row) != 0:
                l_minRowId = min(p_spm.row)
                l_minColId = (min(p_spm.col) // self.parEntries)*self.parEntries
                l_idx = 0
                while l_idx < len(p_spm.row) and (p_spm.row[l_idx] < l_minRowId + self.maxRows):
                    l_idx += 1
                p_rb.row.append(p_spm.row[0:l_idx])
                p_rb.col.append(p_spm.col[0:l_idx])
                p_rb.data.append(p_spm.data[0:l_idx])
                del p_spm.row[:l_idx]
                del p_spm.col[:l_idx]
                del p_spm.data[:l_idx]
                l_maxColId = max(p_rb.col[self.rbParam.totalRbs]) 
                l_rows = len(set(p_rb.row[self.rbParam.totalRbs]))
                l_nnzs = len(p_rb.row[self.rbParam.totalRbs])
                self.rbParam.add_rbIdxInfo(l_minRowId, l_minColId, l_maxColId, l_numPars)
                self.rbParam.add_rbSizeInfo(l_rows, l_nnzs)
                for i in range(6):
                    self.rbParam.add_dummyInfo()
                self.rbParam.totalRbs += 1
        else:
            print("ERROR: cannot sort matrix along rows")
            return

    def pad_par(self, p_row, p_col, p_data):
        l_parNnzs = 0
        l_nnzs = len(p_row)
        l_row,l_col,l_data = [],[],[]
        l_rowNnzs = Counter(p_row)
        l_rows = len(l_rowNnzs)
        while l_nnzs > 0:
            l_rowId = p_row[0]
            l_cRowNnzs = l_rowNnzs[l_rowId]
            l_oneRow = p_row[:l_cRowNnzs]
            l_oneCol = p_col[:l_cRowNnzs]
            l_oneData = p_data[:l_cRowNnzs]
            del p_row[:l_cRowNnzs]
            del p_col[:l_cRowNnzs]
            del p_data[:l_cRowNnzs]
            l_nnzs -= l_cRowNnzs
            
            l_modId = 0
            l_idx = 0
            l_colIdBase = 0
            l_rRowNnzs = 0 
            while l_idx < l_cRowNnzs:
                l_dataItem = l_oneData[l_idx]
                l_colId = l_oneCol[l_idx]
                if l_modId == 0:
                    l_colIdBase = (l_colId // self.parEntries) * self.parEntries
                l_colIdMod = l_colId % self.parEntries
                if (l_colId != l_colIdBase+l_modId):
                    l_row.append(l_rowId)
                    l_col.append(l_colIdBase+l_modId)
                    l_data.append(0)
                else:
                    l_row.append(l_rowId)
                    l_col.append(l_colId)
                    l_data.append(l_dataItem)
                    l_idx += 1
                l_rRowNnzs += 1
                l_modId = (l_modId+1) % self.parEntries
                
            while (l_rRowNnzs % (self.parEntries * self.accLatency) !=0):
                l_row.append(l_rowId)
                l_col.append(l_colIdBase)
                l_data.append(0)
                l_rRowNnzs += 1
            l_rowNnzs[l_rowId] = l_rRowNnzs
            l_parNnzs += l_rRowNnzs
        return [l_rows, l_parNnzs, l_rowNnzs, l_row, l_col, l_data]
 
    def gen_pars(self, p_rb, p_par):
        p_par.totalPars = 0
        l_totalRbs = self.rbParam.totalRbs
        for i in range(l_totalRbs):
            l_rbPars = 0
            l_row=[]
            l_col=[]
            l_data=[]
            l_minColId = self.rbParam.get_rbInfo(i, 0)[1] 
            l_nnzs  = self.rbParam.get_rbInfo(i, 1)[1]
            l_sId,l_eId = 0,0
            while l_nnzs > 0:
                if (l_sId >= len(p_rb.row)):
                    l_minColId = (min(p_rb.col[i]) // self.parEntries) * self.parEntries
                    p_par.row.append(l_row)
                    p_par.col.append(l_col)
                    p_par.data.append(l_data)
                    l_rbPars += 1
                    l_row,l_col,l_data = [],[],[]
                    l_nnzs -= l_parNnzs
                    l_sId, l_eId = 0,0
                else:
                    while l_eId < l_nnzs and p_rb.col[i][l_eId] < l_minColId + self.maxCols:
                        l_eId += 1
                    if (l_eId > l_sId):
                        l_row.extend(p_rb.row[i][l_sId:l_eId])
                        l_col.extend(p_rb.col[i][l_sId:l_eId])
                        l_data.extend(p_rb.data[i][l_sId:l_eId])
                        del p_rb.row[i][l_sId:l_eId]
                        del p_rb.col[i][l_sId:l_eId]
                        del p_rb.data[i][l_sId:l_eId]
                        l_nnzs -=  (l_eId - l_sId)
                        l_sId = l_eId
            p_par.row.append(l_row)
            p_par.col.append(l_col)
            p_par.data.append(l_data)
            l_rbPars += 1
            self.rbParam.set_numPars(i, l_rbPars)
            p_par.totalPars += l_rbPars

    def gen_chPars(self, p_par):
        l_totalPars = p_par.totalPars
        for i in range(l_totalPars):
            l_row = p_par.row[i]
            l_col = p_par.col[i]
            l_data = p_par.data[i]
            [l_rows, l_parNnzs, l_rowNnzs, p_par.row[i], p_par.col[i], p_par.data[i]] = self.pad_par(l_row, l_col, l_data)
            l_row,l_col,l_data = p_par.row[i],p_par.col[i],p_par.data[i]
            l_maxColId, l_minColId = max(p_par.col[i]), min(p_par.col[i])
            if l_parNnzs == 0:
                l_cols = 0
            else:
                l_cols = (l_maxColId+1-l_minColId)
            l_minRowId = l_row[0]
            p_par.minColId.append(l_minColId)
            p_par.minRowId.append(l_minRowId)
            p_par.rows.append(l_rows)
            p_par.cols.append(l_cols)
            p_par.nnzs.append(l_parNnzs)
            l_nnzsPerCh = l_parNnzs // self.channels
            l_minChColId = np.zeros(self.channels, dtype=np.uint32)
            l_minChRowId = np.zeros(self.channels, dtype=np.uint32)
            l_chRows = np.zeros(self.channels, dtype=np.uint32)
            l_chCols = np.zeros(self.channels, dtype=np.uint32)
            l_chNnzs = np.zeros(self.channels, dtype=np.uint32)
            l_sIdx,l_idx = 0,0
            for c in range(self.channels):
                if l_idx < l_parNnzs:
                    l_sIdx = l_idx
                    l_rowId = l_row[l_idx]
                    l_minChRowId[c] = l_rowId
                    l_allocatedNnzs = l_rowNnzs[l_rowId] 
                    l_idx = l_idx + l_rowNnzs[l_rowId]
                    while l_allocatedNnzs < l_nnzsPerCh and l_idx < l_parNnzs:
                        l_rowId = l_row[l_idx] 
                        l_allocatedNnzs = l_allocatedNnzs + l_rowNnzs[l_rowId]
                        l_idx = l_idx + l_rowNnzs[l_rowId]
                    if c == self.channels -1:
                        l_idx = l_parNnzs
                    l_minChColId[c] = min(l_col[l_sIdx:l_idx])
                    l_chNnzs[c] = l_idx - l_sIdx
                    l_chCols[c] = max(l_col[l_sIdx:l_idx])+1-min(l_col[l_sIdx:l_idx])
                    l_chRows[c] = max(l_row[l_sIdx:l_idx])+1-min(l_row[l_sIdx:l_idx])
                else:
                    l_chNnzs[c] = 0
                    l_chCols[c] = 0
                    l_chRows[c] = 0
                    l_minChRowId[c] = 0
                    l_minChColId[c] = 0
            if sum(l_chNnzs) != l_parNnzs:
                print("ERROR: sum of nnzs in channel partitions != nnzs in the partition")
            p_par.minChColId.append(l_minChColId)
            p_par.minChRowId.append(l_minChRowId)
            p_par.chNnzs.append(l_chNnzs)
            p_par.chCols.append(l_chCols)
            p_par.chRows.append(l_chRows) 
    
    def update_rbParams(self, p_par):
        l_totalRbs = self.rbParam.totalRbs
        l_sRbParId = 0
        for rbId in range(l_totalRbs):
            [l_sRbRowId, l_minRbColId, l_maxRbColId, l_rbNumPars] = self.rbParam.get_rbInfo(rbId, 0)[0:4]
            l_chRbMinRowId = []
            l_chRbRows = [0] * self.channels
            l_chRbNnzs = [0] * self.channels
            for c in range(self.channels):
                l_minRowId = p_par.minChRowId[l_sRbParId][c]
                for parId in range(l_rbNumPars):
                    l_parId = l_sRbParId+parId
                    if l_minRowId > p_par.minChRowId[l_parId][c]:
                        l_minRowId = p_par.minChRowId[l_parId][c]
                    l_chRbRows[c] = l_chRbRows[c] + p_par.chRows[l_parId][c]
                    l_chRbNnzs[c] = l_chRbNnzs[c] +  p_par.chNnzs[l_parId][c]

                l_chRbMinRowId.append(l_minRowId-l_sRbRowId)
            self.rbParam.set_chInfo16(rbId, 0, l_chRbMinRowId)
            self.rbParam.set_chInfo16(rbId, 1, l_chRbRows)
            self.rbParam.set_chInfo32(rbId, l_chRbNnzs)                 
            l_sRbParId += l_rbNumPars

    def gen_parParams(self, p_par):
        l_totalPars = p_par.totalPars
        self.parParam.totalPars = l_totalPars
        for parId in range(l_totalPars):
            self.parParam.add_chInfo32(p_par.chCols[parId])
            self.parParam.add_chInfo32(p_par.chNnzs[parId])
            l_baseColAddr = p_par.minColId[parId] // self.parEntries
            l_colBks = p_par.cols[parId] // self.parEntries
            l_rows = p_par.rows[parId]
            l_nnzs = p_par.nnzs[parId]
            self.parParam.add_parInfo(l_baseColAddr, l_colBks, l_rows, l_nnzs)
            l_chBaseAddr = []
            for c in range(self.channels):
                l_chBaseAddr.append(p_par.minChColId[parId][c] // self.parEntries - l_baseColAddr)
            self.parParam.add_chInfo16(l_chBaseAddr)
            self.parParam.add_chInfo16(p_par.chCols[parId] // self.parEntries)
            self.parParam.add_dummyInfo()

    def gen_nnzStore(self, p_par):
        l_memIdxWidth = self.memBits//16
        l_rowIdxGap = self.parEntries * self.accLatency
        l_rowIdxMod = l_memIdxWidth * l_rowIdxGap
        l_colIdxMod = l_memIdxWidth * self.parEntries
        l_sParId = 0
        for rbId in range(self.rbParam.totalRbs):
            l_pars = self.rbParam.get_rbInfo(rbId, 0)[3]
            l_sRbRowId = self.rbParam.get_rbInfo(rbId, 0)[0]
            for parId in range(l_pars):
                l_parId = l_sParId + parId
                l_sChIdx = 0
                l_sParColId = self.parParam.get_parInfo(l_parId)[0]
                for c in range(self.channels):
                    l_totalBks = 0
                    l_totalRowIdxBks = 0
                    l_totalColIdxBks = 0
                    l_totalNnzBks = 0
                    l_rowIdx,l_colIdx =  [],[]
                    l_sChRbRowId = self.rbParam.get_chInfo16(rbId, 0)[c]
                    l_sChParColId = self.parParam.get_chInfo16(l_parId, 0)[c]

                    for i in range(0, p_par.chNnzs[l_parId][c], self.parEntries):
                        if i % l_rowIdxMod == 0:
                            for j in range(l_memIdxWidth):
                                if i+j*l_rowIdxGap < p_par.chNnzs[l_parId][c]:
                                    l_rowIdx.append(p_par.row[l_parId][l_sChIdx+i+j*l_rowIdxGap] - l_sRbRowId - l_sChRbRowId)
                                else:
                                    l_rowIdx.append(0)
                            self.nnzStore.add_idxArr(c, l_rowIdx)
                            l_totalRowIdxBks += 1
                        if i % l_colIdxMod == 0:
                            for j in range(l_memIdxWidth):
                                if i+j*self.parEntries < p_par.chNnzs[l_parId][c]:
                                    l_colIdx.append(p_par.col[l_parId][l_sChIdx+i+j*self.parEntries]//self.parEntries-l_sParColId-l_sChParColId)
                                else:
                                    l_colIdx.append(0)
                            self.nnzStore.add_idxArr(c,l_colIdx)
                            l_totalColIdxBks += 1
                        l_nnz = p_par.data[l_parId][l_sChIdx+i:l_sChIdx+i+self.parEntries]
                        self.nnzStore.add_nnzArr(c,l_nnz)
                        l_totalNnzBks += 1
                    l_sChIdx += p_par.chNnzs[l_parId][c]
                    l_totalBks = l_totalNnzBks + l_totalRowIdxBks + l_totalColIdxBks
                    self.nnzStore.totalBks[c] += l_totalBks
                    self.nnzStore.totalRowIdxBks[c] += l_totalRowIdxBks
                    self.nnzStore.totalColIdxBks[c] += l_totalColIdxBks
                    self.nnzStore.totalNnzBks[c] += l_totalNnzBks
                p_par.row[l_parId] = []
                p_par.col[l_parId] = []
                p_par.data[l_parId] = []
            l_sParId += l_pars

    def process(self, p_spm):
        l_rb = row_block()
        l_par = partition()
        self.gen_rbs(p_spm, l_rb)
        self.gen_pars(l_rb, l_par)
        self.gen_chPars(l_par)
        self.update_rbParams(l_par)
        self.gen_parParams(l_par)
        self.gen_nnzStore(l_par)

    def store_rbParam(self, fileName):
        self.rbParam.write_file(fileName)

    def store_parParam(self, fileName):
        self.parParam.write_file(fileName)

    def store_nnz(self, fileNames):
        self.nnzStore.write_file(fileNames)

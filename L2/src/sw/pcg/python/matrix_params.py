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
import scipy.io as sio
import scipy.sparse as sp
import math
import utils as ut

clas row_block:
    def __init__(self):
        self.row = []
        self.col = []
        self.data = []

class row_block_param:
    def __init__(self, memBits, channels):
        self.channels = channels
        self.memBytes = memBits // 8
        self.totalRows = 0
        self.totalRbs = 0
        self.buf = bytearray()
        self.int32Arr = np.zeros(memBits//32, dtype=np.uint32)
        self.chInfo16Arr = np.zeros(channels, dtype=np.uint16)
        self.chInfo32Arr = np.zeros(channels, dtype=np.uint32)
        self.int32Arr[0] = self.totalRows
        self.int32Arr[1] = self.totalRbs
        self.buf.extend(self.int32Arr.tobytes())

    def add_rbIdxInfo(self, p_minRowId, p_minColId, p_maxColId, p_numPars):
        self.int32Arr[0:4] = [p_minRowId, p_minColId, p_maxColId, p_numPars]
        self.buf.extend(self.int32Arr.tobytes())

    def add_rbSizeInfo(self, p_numRows, p_numNnzs):
        self.int32Arr[0:2] = [p_numRows, p_numNnzs]
        self.buf.extend(self.int32Arr.tobytes())

    def add_dummyInfo(self):
        self.buf.extend(self.int32Arr.tobytes())

    def add_chInfo16(self, p_info):
        for i in range(self.channels):
            self.chInfo16Arr[i] = p_info[i]
        self.buf.extend(self.chInfo16Arr.tobytes())

    def add_chInfo32(self, p_info):
        for i in range(self.channels):
            self.chInfo32Arr[i] = p_info[i]
        self.buf.extend(self.chInfo32Arr.tobytes())
        
    def get_rb_offset(self, p_rbId):
        l_offset = self.memBytes
        l_offset += p_rbId * (self.memBytes*8)
        return l_offset
        
    def get_rbInfo(self, p_rbId, p_rbInfoId):
        l_size = self.memBytes // 4
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += p_rbInfoId*self.memBytes
        l_infoArr = np.frombuffer(self.buf, dtype=np.uint32, count=l_size, offset=l_offset)
        return l_infoArr
    
    def set_minMaxColId(self, p_rbId, p_minColId, p_maxColId):
        l_offset = self.get_rb_offset(p_rbId)
        self.int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes // 4, offset=l_offset)
        self.int32Arr[1:3] = [p_minColId, p_maxColId]
        self.buf[l_offset : l_offset+self.memBytes] = self.int32Arr.tobytes()

    def set_numPars(self, p_rbId, p_numPars):
        l_offset = self.get_rb_offset(p_rbId)
        self.int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes // 4, offset=l_offset)
        self.int32Arr[3] = p_numPars
        self.buf[l_offset : l_offset+self.memBytes] = self.int32Arr.tobytes()

    def set_numNnzs(self, p_rbId, p_numNnzs):
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += self.memBytes
        self.int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes // 4, offset=l_offset)
        self.int32Arr[1] = p_numNnzs
        self.buf[l_offset : l_offset+self.memBytes] = self.int32Arr.tobytes()

    def get_chInfo16(self, p_rbId, p_chInfo16Id):
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += self.memBytes * (2 + p_chInfo16Id)
        l_chInfo16 = np.frombuffer(self.buf, dtype=np.uint16, count=self.channels, offset=l_offset)
        return l_chInfo16

    def set_chInfo16(self, p_rbId, p_chInfo16Id, p_info):
        for i in range(self.channels):
            self.chInfo16Arr[i] = p_info[i]
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += self.memBytes * (2 + p_chInfo16Id)
        self.buf[l_offset:l_offset+self.channels*2] = self.chInfo16Arr.tobytes()

    def get_chInfo32(self, p_rbId):
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += self.memBytes * 4
        l_chInfo32 = np.frombuffer(self.buf, dtype=np.uint32, count=self.channels, offset=l_offset)
        return l_chInfo32

    def set_chInfo32(self, p_rbId, p_info):
        for i in range(self.channels):
            self.chInfo32Arr[i] = p_info[i]
        l_offset = self.get_rb_offset(p_rbId)
        l_offset += self.memBytes * 4
        self.buf[l_offset:l_offset+self.channels*4] = self.chInfo32Arr.tobytes()

    def write_file(self, fileName):
        fo = open(fileName, "wb")
        self.int32Arr[0] = self.totalRows
        self.int32Arr[1] = self.totalRbs
        self.buf[:self.memBytes] = self.int32Arr.tobytes()
        fo.write(self.buf)
        fo.close()

    def read_file(self, filename):
        fi = open(fileName, "rb")
        self.buf = fi.read()
        self.int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes // 4, offset=0)
        self.totalRows = self.int32Arr[0]
        self.totalRbs = self.int32Arr[1]
        fi.close()

class partition:
    def __init__(self):
        self.row = []
        self.col = []
        self.data = []

class par_param:
    def __init__(self, memBits, channels):
        self.memBytes = memBits//8
        self.channels = channels
        self.totalPars = 0
        self.buf = bytearray()
        self.int32Arr = np.zeros(self.memBytes//4, dtype=np.uint32)
        self.chInfo16Arr = np.zeros(self.channels, dtype=np.uint16)
        self.chInfo32Arr = np.zeros(self.channels, dtype=np.uint16)
        self.int32Arr[0] = self.totalPars
        self.buf.extend(self.int32Arr.tobytes)
    
    def add_chInfo16(self, p_info):
        for i in range(self.channels):
            self.chInfo16Arr[i] = p_info[i]
        self.buf.extend(self.chInfo16Arr.tobytes())

    def add_chInfo32(self, p_info):
        for i in range(self.channels):
            self.chInfo32Arr[i] = p_info[i]
        self.buf.extend(self.chInfo32Arr.tobytes())

    def add_parInfo(self, p_info):
        for i in range(self.memBytes // 4):
            self.int32Arr[i] = p_info[i]
        self.buf.extend(self.int32Arr.tobytes())

    def get_par_offset(self, p_parId):
        l_offset = self.memBytes + p_parId * 8 * self.memBytes
        return l_offset

    def get_chInfo16(self, p_parId, p_chInfo16Id):
        l_offset = self.get_par_offset(p_parId)
        l_offset += (5+p_chInfo16Id)*self.memBytes
        l_chInfo16 = np.frombuffer(self.buf, dtype=np.uint16, count=self.channels, offset=l_offset)
        return l_chInfo16

    def set_chInfo16(self, p_parId, p_chInfo16Id, p_info);
        for i in range(self.channels):
            self.chInfo16Arr[i] = p_info[i]
        l_offset = self.get_par_offset(p_parId)
        l_offset += (5+p_chInfo16Id)*self.memBytes
        self.buf[l_offset:l_offset+2*self.channels] = self.chInfo16Arr.tobytes()

    def get_chInfo32(self, p_parId, p_chInfo32Id):
        l_offset = self.get_par_offset(p_parId)
        l_offset += p_chInfo32Id * self.memBytes
        l_chInfo32 = np.frombuffer(self.buf, dtype=np.uint32, count=self.channels, offset=l_offset)
        return l_chInfo32

    def set_chInfo32(self, p_parId, p_chInfo32Id, p_info):
        for i in range(self.channels):
            self.chInfo32Arr[i] = p_info[i]
        l_offset = self.get_par_offset(p_parId)
        l_offset += p_chInfo32Id * self.memBytes
        self.buf[l_offset:l_offset+4*self.channels] = self.chInfo32Arr.tobytes()

    def get_parInfo(self, p_parId):
        l_offset = self.get_par_offset(p_parId)
        l_offset += 4*self.memBytes
        l_int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes // 4, offset=l_offset)
        return l_int32Arr

    def set_parInfo(self, p_parId, p_info):
        for i in range(self.memBytes // 4):
            self.int32Arr[i] = p_info[i]
        l_offset = self.get_par_offset(p_parId)
        l_offset += 4*self.memBytes
        self.buf[l_offset: l_offset+self.memBytes] = sef.int32Arr.tobytes()


    def write_file(self, filename):
        self.int32Arr[0] = self.totalPars
        self.buf[:self.memBytes] = self.int32Arr.tobytes()
        fo = open(filename, "wb")
        fo.write(self.buf)
        fo.close() 

    def read_file(self, filename):
        fi = open(filename, "rb")
        self.buf = fi.read()
        self.int32Arr = np.frombuffer(self.buf, dtype=np.uint32, count=self.memBytes//4, offset=0)
        self.totalPars = self.int32Arr[0]
        fi.close()

class sparse_matrix:
    def __init__(self):
      pass
    
    def read_matrix(self, mtxFullName, mtxName):
        self.mat = sio.mmread(mtxFullName)
        if sp.issparse(mat):
            mat.eliminate_zeros()
            self.mtxName = mtxName
            self.m,self.n = mat.shape
            self.nnz = self.mat.nnz
            return True
        else:
            return False

    def sort(self, p_order):
        l_res = ut.sort_coo(self.mat.row, self.mat.col, self.mat.data, p_order)
        return l_res

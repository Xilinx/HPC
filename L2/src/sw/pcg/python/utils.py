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

def open_files(p_fileNames, mode):
    l_files = []
    for l_fileName in p_fileNames:
        l_files.append(open(l_fileName, mode))
    return l_files

def close_files(p_files):
    for l_file in p_files:
        l_file.close()

def sort_coo(p_row, p_col, p_data, p_order):
    if p_order =='r':
        order = np.lexsort((p_col, p_row))
    elif p_order == 'c':
        order = np.lexsort((p_row, p_col))
    else:
        print("ERROR: order input must be \'r\' or \'c\'")
        return False
    p_row = p_row[order]
    p_col = p_col[order]
    p_data = p_data[order]
    return True

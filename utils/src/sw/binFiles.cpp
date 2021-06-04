/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include "binFiles.hpp"
using namespace std;

size_t getBinBytes(const string filename) {
    ifstream file(filename, ios::binary);
    file.unsetf(ios::skipws);
    streampos fileSize;
    file.seekg(0, ios::end);
    fileSize = file.tellg();
    file.close();
    return fileSize;
}


void readBin(string name, void* mat, unsigned int totalSize) {
    ifstream inFile;
    inFile.open(name, ifstream::binary);
    if (inFile.is_open()) {
        inFile.read((char*)mat, totalSize);
        inFile.close();
    } else {
        cerr << "Could not find " << name << endl;
        exit(1);
    }
}

void saveBin(string name, void* mat, unsigned int totalSize) {
    ofstream outFile(name, ios::binary);
    outFile.write((char*)mat, totalSize);
}

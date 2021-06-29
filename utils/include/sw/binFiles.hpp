/*
 * Copyright 2019-2021 Xilinx, Inc.
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
#ifndef __BINFILES_HPP_
#define __BINFILES_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

inline
size_t getBinBytes(const std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.close();
    return fileSize;
}

template <typename T>
std::vector<T> readBin(const std::string filename) {
    std::vector<T> vec;
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    const std::streampos vecSize = fileSize / sizeof(T);
    vec.resize(vecSize);

    file.read(reinterpret_cast<char*>(vec.data()), fileSize);
    file.close();
    return vec;
}

template <typename T, typename A>
bool readBin(const std::string filename, const std::streampos readSize, std::vector<T, A>& vec) {
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (readSize > 0 && fileSize != readSize) {
        std::cout << "WARNNING: file " << filename << " size " << fileSize << " doesn't match required size " << readSize
             << std::endl;
    }
    assert(fileSize >= readSize);

    const std::streampos vecSize = fileSize / sizeof(T);
    vec.resize(vecSize);

    file.read(reinterpret_cast<char*>(vec.data()), fileSize);
    file.close();
    if (file)
        return true;
    else
        return false;
}

template <typename T>
bool readBin(const std::string filename, const std::streampos readSize, T* vec) {
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (readSize > 0 && fileSize != readSize) {
        std::cout << "WARNNING: file " << filename << " size " << fileSize << " doesn't match required size " << readSize
             << std::endl;
    }
    assert(fileSize >= readSize);

    file.read(reinterpret_cast<char*>(vec), fileSize);
    file.close();
    if (file)
        return true;
    else
        return false;
}

template <typename T>
bool writeBin(const std::string filename, const std::streampos writeSize, const T* vec) {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(vec), writeSize);
    file.close();
    if (file)
        return true;
    else
        return false;
}

template <typename T, typename A>
bool writeBin(const std::string filename, const std::streampos writeSize, std::vector<T, A>& vec) {
    std::streampos fileSize = vec.size() * sizeof(T);
    if (writeSize > 0 && fileSize != writeSize) {
        std::cout << "WARNNING: std::vector size " << fileSize << " doesn't match required size " << writeSize << std::endl;
    }
    assert(fileSize >= writeSize);

    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<char*>(vec.data()), writeSize);
    file.close();
    if (file)
        return true;
    else
        return false;
}

inline
void readBin(std::string name, void* mat, unsigned int totalSize) {
    std::ifstream inFile;
    inFile.open(name, std::ifstream::binary);
    if (inFile.is_open()) {
        inFile.read((char*)mat, totalSize);
        inFile.close();
    } else {
        std::cerr << "Could not find " << name << std::endl;
        exit(1);
    }
}

inline
void saveBin(std::string name, void* mat, unsigned int totalSize) {
    std::ofstream outFile(name, std::ios::binary);
    outFile.write((char*)mat, totalSize);
}
#endif

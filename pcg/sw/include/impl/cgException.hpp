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

#ifndef PCG_EXCEPTION_HPP
#define PCG_EXCEPTION_HPP

namespace xilinx_apps {
namespace pcg {

class CgException: public std::exception {
    public:
        CgException(std::string str): msg(str){
        }

        std::string getMessage(){
            return msg;
        }

    private: 
        std::string msg;
};

class CgInternalError: public CgException {
    public:
        CgInternalError(std::string str): CgException("INTERNAL ERROR: "+str){}
};

class CgAllocFailed: public CgException  {
    public:
        CgAllocFailed(std::string str): CgException("Alloc Failed ERROR: "+str){}
};

class CgInvalidValue: public CgException  {
    public:
        CgInvalidValue(std::string str): CgException("INVALID VALUE ERROR: "+str){}
};

}
}
#endif


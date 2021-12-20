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

#ifndef MPIDEFS_HPP
#define MPIDEFS_HPP

namespace xilinx_apps {
namespace xans {

typedef enum {
    CTRL = 0x01,
    SYNC = 0x02,
    ACK = 0x04,
    FIN = 0x10,
    INT_CTRL = 0x20,
    LAST = 0x40,
    DATA = 0x80
} MPI_PKT_TYPE;

typedef enum {
    MPI_SEND,
    MPI_RECEIVE,
    MPI_SEND_CTRL,
    MPI_RECEIVE_CTRL,
    MPI_FIN
} MPI_OPCODE;

typedef enum {
    CTRL_NORM,
    CTRL_SYNC,
    CTRL_ACK,
    CTRL_SEND,
    CTRL_RECEIVE,
    CTRL_INT,
    CTRL_LOOP_START,
    CTRL_LOOP_END,
    CTRL_FIN
} MPI_MSG_TAG;

}
}
#endif

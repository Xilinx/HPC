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

/**
 * @file xf_blas.hpp
 * @brif Top-level header for Vitis BLAS Library.
 *
 */

#ifndef XF_BLAS_HPP
#define XF_BLAS_HPP

// shared modules
#include "ap_int.h"
#include "hls_stream.h"
#include "blas/helpers.hpp"

// BLAS L1 function modules

#include "blas/amax.hpp"
#include "blas/amin.hpp"
#include "blas/asum.hpp"
#include "blas/axpy.hpp"
#include "blas/copy.hpp"
#include "blas/dot.hpp"
#include "blas/scal.hpp"
#include "blas/swap.hpp"
#include "blas/nrm2.hpp"

// BLAS L2 function modules

#include "blas/gemv.hpp"
#include "blas/gbmv.hpp"
#include "blas/symv.hpp"
#include "blas/trmv.hpp"
/* TODO
 *
 */

// BLAS L3 function modules

#include "blas/gemm.hpp"

/* TODO
 *
 *
*/

#endif

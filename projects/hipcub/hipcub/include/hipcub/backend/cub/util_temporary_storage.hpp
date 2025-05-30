/******************************************************************************
 * Copyright (c) 2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2024, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright (c) 2024-2025, Advanced Micro Devices, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef HIPCUB_CUB_UTIL_TEMPORARY_STORAGE_HPP_
#define HIPCUB_CUB_UTIL_TEMPORARY_STORAGE_HPP_

#include "../../config.hpp"

#include <cub/util_temporary_storage.cuh>

BEGIN_HIPCUB_NAMESPACE

/// \brief Alias temporaries to externally-allocated device storage (or simply return the amount of storage needed).
/// \tparam ALLOCATIONS The number of allocations that are needed.
/// \param d_temp_storage [in] Device-accessible allocation of temporary storage.  When nullptr, the required allocation size is written to \p temp_storage_bytes and no work is done.
/// \param temp_storage_bytes [in,out] Size in bytes of \t d_temp_storage allocation.
/// \param allocations [out] Pointers to device allocations needed.
/// \param allocation_sizes [in] Sizes in bytes of device allocations needed.
template<int ALLOCATIONS>
HIPCUB_HOST_DEVICE
HIPCUB_FORCEINLINE hipError_t AliasTemporaries(void*   d_temp_storage,
                                               size_t& temp_storage_bytes,
                                               void* (&allocations)[ALLOCATIONS],
                                               const size_t (&allocation_sizes)[ALLOCATIONS])
{
    cudaError_t error = ::cub::AliasTemporaries(d_temp_storage,
                                                temp_storage_bytes,
                                                allocations,
                                                allocation_sizes);

    if(cudaSuccess == error)
    {
        return hipSuccess;
    }
    else if(cudaErrorInvalidValue == error)
    {
        return hipErrorInvalidValue;
    }

    return hipErrorUnknown;
}

END_HIPCUB_NAMESPACE

#endif // HIPCUB_CUB_UTIL_TEMPORARY_STORAGE_HPP_

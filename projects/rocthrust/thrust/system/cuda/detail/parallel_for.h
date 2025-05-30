/******************************************************************************
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#pragma once

#include <thrust/detail/config.h>

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_NVCC

#include <thrust/detail/type_traits/result_of_adaptable_function.h>
#include <thrust/system/cuda/config.h>
#include <thrust/system/cuda/detail/cdp_dispatch.h>
#include <thrust/system/cuda/detail/core/agent_launcher.h>
#include <thrust/system/cuda/detail/par_to_seq.h>
#include <thrust/system/cuda/detail/util.h>

THRUST_NAMESPACE_BEGIN

namespace cuda_cub {

namespace __parallel_for {

  template <int _BLOCK_THREADS,
            int _ITEMS_PER_THREAD = 1>
  struct PtxPolicy
  {
    enum
    {
      BLOCK_THREADS    = _BLOCK_THREADS,
      ITEMS_PER_THREAD = _ITEMS_PER_THREAD,
      ITEMS_PER_TILE   = BLOCK_THREADS * ITEMS_PER_THREAD,
    };
  };    // struct PtxPolicy

  template <class Arch, class F>
  struct Tuning;

  template <class F>
  struct Tuning<sm30, F>
  {
    using type = PtxPolicy<256, 2>;
  };


  template <class F,
            class Size>
  struct ParallelForAgent
  {
    template <class Arch>
    struct PtxPlan : Tuning<Arch, F>::type
    {
      using tuning = Tuning<Arch, F>;
    };
    using ptx_plan = core::specialize_plan<PtxPlan>;

    enum
    {
      ITEMS_PER_THREAD = ptx_plan::ITEMS_PER_THREAD,
      ITEMS_PER_TILE   = ptx_plan::ITEMS_PER_TILE,
      BLOCK_THREADS    = ptx_plan::BLOCK_THREADS
    };

    template <bool IS_FULL_TILE>
    static void    THRUST_DEVICE_FUNCTION
    consume_tile(F    f,
                 Size tile_base,
                 int  items_in_tile)
    {
#pragma unroll
      for (int ITEM = 0; ITEM < ITEMS_PER_THREAD; ++ITEM)
      {
        Size idx = BLOCK_THREADS * ITEM + threadIdx.x;
        if (IS_FULL_TILE || idx < items_in_tile)
          f(tile_base + idx);
      }
    }

    THRUST_AGENT_ENTRY(F     f,
                       Size  num_items,
                       char * /*shmem*/ )
    {
      Size tile_base     = static_cast<Size>(blockIdx.x) * ITEMS_PER_TILE;
      Size num_remaining = num_items - tile_base;
      Size items_in_tile = static_cast<Size>(
          num_remaining < ITEMS_PER_TILE ? num_remaining : ITEMS_PER_TILE);

      if (items_in_tile == ITEMS_PER_TILE)
      {
        // full tile
        consume_tile<true>(f, tile_base, ITEMS_PER_TILE);
      }
      else
      {
        // partial tile
        consume_tile<false>(f, tile_base, items_in_tile);
      }
    }
  };    // struct ParallelForEagent

  template <class F,
            class Size>
  THRUST_RUNTIME_FUNCTION cudaError_t
  parallel_for(Size         num_items,
               F            f,
               cudaStream_t stream)
  {
    if (num_items == 0)
      return cudaSuccess;
    using core::AgentLauncher;
    using core::AgentPlan;

    using parallel_for_agent = AgentLauncher<ParallelForAgent<F, Size> >;
    AgentPlan parallel_for_plan = parallel_for_agent::get_plan(stream);

    parallel_for_agent pfa(parallel_for_plan, num_items, stream, "transform::agent");
    pfa.launch(f, num_items);
    CUDA_CUB_RET_IF_FAIL(cudaPeekAtLastError());

    return cudaSuccess;
  }
}    // __parallel_for

_CCCL_EXEC_CHECK_DISABLE
template <class Derived,
          class F,
          class Size>
void _CCCL_HOST_DEVICE
parallel_for(execution_policy<Derived> &policy,
             F                          f,
             Size                       count)
{
  if (count == 0)
  {
    return;
  }

  // clang-format off
  THRUST_CDP_DISPATCH(
    (cudaStream_t stream = cuda_cub::stream(policy);
     cudaError_t  status = __parallel_for::parallel_for(count, f, stream);
     cuda_cub::throw_on_error(status, "parallel_for failed");
     status = cuda_cub::synchronize_optional(policy);
     cuda_cub::throw_on_error(status, "parallel_for: failed to synchronize");),
    // CDP sequential impl:
    (for (Size idx = 0; idx != count; ++idx)
     {
       f(idx);
     }
  ));
  // clang-format on
}

}    // namespace cuda_cub

THRUST_NAMESPACE_END
#endif

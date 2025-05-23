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
#include <thrust/distance.h>
#include <thrust/system/cuda/config.h>
#include <thrust/system/cuda/execution_policy.h>
#include <thrust/system/cuda/detail/parallel_for.h>
#include <thrust/distance.h>

THRUST_NAMESPACE_BEGIN
namespace cuda_cub {

namespace __tabulate {

  template <class Iterator, class TabulateOp, class Size>
  struct functor
  {
    Iterator items;
    TabulateOp op;

    _CCCL_HOST_DEVICE
    functor(Iterator items_, TabulateOp op_)
        : items(items_), op(op_) {}

    void _CCCL_DEVICE operator()(Size idx)
    {
      items[idx] = op(idx);
    }
  };    // struct functor

}    // namespace __tabulate

template <class Derived,
          class Iterator,
          class TabulateOp>
void _CCCL_HOST_DEVICE
tabulate(execution_policy<Derived>& policy,
         Iterator                   first,
         Iterator                   last,
         TabulateOp                 tabulate_op)
{
  using size_type = typename iterator_traits<Iterator>::difference_type;

  size_type count = thrust::distance(first, last);

  using functor_t = __tabulate::functor<Iterator, TabulateOp, size_type>;

  cuda_cub::parallel_for(policy,
                         functor_t(first, tabulate_op),
                         count);
}

}    // namespace cuda_cub
THRUST_NAMESPACE_END
#endif

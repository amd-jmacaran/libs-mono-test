/******************************************************************************
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright© 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
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

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP
#include <iterator>
// This include is not required
// #include <thrust/system/hip/detail/transform.h>
#include <thrust/distance.h>
#include <thrust/swap.h>
#include <thrust/system/hip/detail/par_to_seq.h>
#include <thrust/system/hip/detail/parallel_for.h>

THRUST_NAMESPACE_BEGIN

namespace hip_rocprim
{
namespace __swap_ranges
{
    template <class ItemsIt1, class ItemsIt2>
    struct swap_f
    {
        ItemsIt1 items1;
        ItemsIt2 items2;

        using value1_type = typename iterator_traits<ItemsIt1>::value_type;
        using value2_type = typename iterator_traits<ItemsIt2>::value_type;

        THRUST_HIP_FUNCTION
        swap_f(ItemsIt1 items1_, ItemsIt2 items2_)
            : items1(items1_)
            , items2(items2_)
        {
        }

        template <class Size>
        void THRUST_HIP_DEVICE_FUNCTION operator()(Size idx)
        {
            value1_type item1 = items1[idx];
            value2_type item2 = items2[idx];
            // XXX thrust::swap is buggy
            // if reference_type of ItemIt1/ItemsIt2
            // is a proxy reference, then KABOOM!
            // to avoid this, just copy the value first before swap
            // *todo* specialize on real & proxy references
            using thrust::swap;
            swap(item1, item2);
            items1[idx] = item1;
            items2[idx] = item2;
        }
    };
} // namespace __swap_ranges

template <class Derived, class ItemsIt1, class ItemsIt2>
ItemsIt2 THRUST_HIP_FUNCTION
swap_ranges(execution_policy<Derived>& policy,
            ItemsIt1                   first1,
            ItemsIt1                   last1,
            ItemsIt2                   first2)
{
    using size_type = typename iterator_traits<ItemsIt1>::difference_type;

    size_type num_items = static_cast<size_type>(thrust::distance(first1, last1));

    hip_rocprim::parallel_for(
        policy, __swap_ranges::swap_f<ItemsIt1, ItemsIt2>(first1, first2), num_items
    );
    return first2 + num_items;
}

} // namespace hip_rocprim

THRUST_NAMESPACE_END
#endif

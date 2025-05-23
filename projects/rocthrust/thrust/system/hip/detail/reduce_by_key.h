/******************************************************************************
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright (c) 2019-2025, Advanced Micro Devices, Inc.  All rights reserved.
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

#include <thrust/detail/alignment.h>
#include <thrust/detail/minmax.h>
#include <thrust/detail/raw_reference_cast.h>
#include <thrust/detail/temporary_array.h>
#include <thrust/detail/type_traits/iterator/is_output_iterator.h>
#include <thrust/device_vector.h>
#include <thrust/distance.h>
#include <thrust/functional.h>
#include <thrust/pair.h>
#include <thrust/system/hip/config.h>
#include <thrust/system/hip/detail/get_value.h>
#include <thrust/system/hip/detail/general/temp_storage.h>
#include <thrust/system/hip/detail/par_to_seq.h>
#include <thrust/system/hip/detail/util.h>

#include <cstdint>

// rocprim include
#include <rocprim/rocprim.hpp>

THRUST_NAMESPACE_BEGIN

template <typename DerivedPolicy,
          typename InputIterator1,
          typename InputIterator2,
          typename OutputIterator1,
          typename OutputIterator2,
          typename BinaryPredicate>
THRUST_HOST_DEVICE thrust::pair<OutputIterator1, OutputIterator2>
reduce_by_key(const thrust::detail::execution_policy_base<DerivedPolicy>& exec,
              InputIterator1                                              keys_first,
              InputIterator1                                              keys_last,
              InputIterator2                                              values_first,
              OutputIterator1                                             keys_output,
              OutputIterator2                                             values_output,
              BinaryPredicate                                             binary_pred);

namespace hip_rocprim
{
namespace __reduce_by_key
{
    template <typename Derived,
              typename KeysInputIt,
              typename ValuesInputIt,
              typename UniqueOutputIt,
              typename AggregatesOutputIt,
              typename UniqueCountOutputIt,
              typename BinaryFunction,
              typename KeyCompareFunction>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_reduce_by_key(execution_policy<Derived>& policy,
                                                          void*               temporary_storage,
                                                          size_t&             storage_size,
                                                          KeysInputIt         keys_input,
                                                          ValuesInputIt       values_input,
                                                          const size_t        size,
                                                          UniqueOutputIt      unique_output,
                                                          AggregatesOutputIt  aggregates_output,
                                                          UniqueCountOutputIt unique_count_output,
                                                          BinaryFunction      reduce_op,
                                                          KeyCompareFunction  key_compare_op,
                                                          const hipStream_t   stream,
                                                          bool                debug_sync)
        -> std::enable_if_t<decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::reduce_by_key(temporary_storage,
                                      storage_size,
                                      keys_input,
                                      values_input,
                                      size,
                                      unique_output,
                                      aggregates_output,
                                      unique_count_output,
                                      reduce_op,
                                      key_compare_op,
                                      stream,
                                      debug_sync);
    }

    template <typename Derived,
              typename KeysInputIt,
              typename ValuesInputIt,
              typename UniqueOutputIt,
              typename AggregatesOutputIt,
              typename UniqueCountOutputIt,
              typename BinaryFunction,
              typename KeyCompareFunction>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_reduce_by_key(execution_policy<Derived>& policy,
                                                          void*               temporary_storage,
                                                          size_t&             storage_size,
                                                          KeysInputIt         keys_input,
                                                          ValuesInputIt       values_input,
                                                          const size_t        size,
                                                          UniqueOutputIt      unique_output,
                                                          AggregatesOutputIt  aggregates_output,
                                                          UniqueCountOutputIt unique_count_output,
                                                          BinaryFunction      reduce_op,
                                                          KeyCompareFunction  key_compare_op,
                                                          const hipStream_t   stream,
                                                          bool                debug_sync)
        -> std::enable_if_t<!decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::deterministic_reduce_by_key(temporary_storage,
                                                    storage_size,
                                                    keys_input,
                                                    values_input,
                                                    size,
                                                    unique_output,
                                                    aggregates_output,
                                                    unique_count_output,
                                                    reduce_op,
                                                    key_compare_op,
                                                    stream,
                                                    debug_sync);
    }

    template <typename Derived,
              typename KeysInputIt,
              typename ValuesInputIt,
              typename KeysOutputIt,
              typename ValuesOutputIt,
              typename EqualityOp,
              typename ReductionOp>
    THRUST_HIP_RUNTIME_FUNCTION
    pair<KeysOutputIt, ValuesOutputIt>
    reduce_by_key(execution_policy<Derived>& policy,
                  KeysInputIt                keys_first,
                  KeysInputIt                keys_last,
                  ValuesInputIt              values_first,
                  KeysOutputIt               keys_output,
                  ValuesOutputIt             values_output,
                  EqualityOp                 equality_op,
                  ReductionOp                reduction_op)
    {
        using namespace thrust::system::hip_rocprim::temp_storage;

        using size_type = size_t;
        size_type   num_items = static_cast<size_type>(thrust::distance(keys_first, keys_last));
        size_t      temp_storage_bytes = 0;
        hipStream_t stream             = hip_rocprim::stream(policy);
        bool        debug_sync         = THRUST_HIP_DEBUG_SYNC_FLAG;

        if(num_items == 0)
            return thrust::make_pair(keys_output, values_output);

        hip_rocprim::throw_on_error(invoke_reduce_by_key(policy,
                                                         nullptr,
                                                         temp_storage_bytes,
                                                         keys_first,
                                                         values_first,
                                                         num_items,
                                                         keys_output,
                                                         values_output,
                                                         static_cast<size_type*>(nullptr),
                                                         reduction_op,
                                                         equality_op,
                                                         stream,
                                                         debug_sync),
                                    "reduce_by_key failed on 1st step");

        size_t     storage_size;
        void*      ptr       = nullptr;
        void*      temp_stor = nullptr;
        size_type* d_num_runs_out;

        auto l_part = make_linear_partition(make_partition(&temp_stor, temp_storage_bytes),
                                            ptr_aligned_array(&d_num_runs_out, 1));

        // Calculate storage_size including alignment
        hip_rocprim::throw_on_error(partition(ptr, storage_size, l_part));

        // Allocate temporary storage.
        thrust::detail::temporary_array<std::uint8_t, Derived> tmp(policy, storage_size);
        ptr = static_cast<void*>(tmp.data().get());

        // Create pointers with alignment
        hip_rocprim::throw_on_error(partition(ptr, storage_size, l_part));

        hip_rocprim::throw_on_error(invoke_reduce_by_key(policy,
                                                         ptr,
                                                         temp_storage_bytes,
                                                         keys_first,
                                                         values_first,
                                                         num_items,
                                                         keys_output,
                                                         values_output,
                                                         d_num_runs_out,
                                                         reduction_op,
                                                         equality_op,
                                                         stream,
                                                         debug_sync),
                                    "reduce_by_key failed on 2nd step");

        const auto num_runs_out = hip_rocprim::get_value(policy, d_num_runs_out);

        return thrust::make_pair(keys_output + num_runs_out, values_output + num_runs_out);
    }

} // namespace __reduce_by_key

//-------------------------
// Thrust API entry points
//-------------------------

THRUST_EXEC_CHECK_DISABLE template <class Derived,
                                        class KeyInputIt,
                                        class ValInputIt,
                                        class KeyOutputIt,
                                        class ValOutputIt,
                                        class BinaryPred,
                                        class BinaryOp>
THRUST_HIP_FUNCTION
pair<KeyOutputIt, ValOutputIt>
reduce_by_key(execution_policy<Derived>& policy,
              KeyInputIt                 keys_first,
              KeyInputIt                 keys_last,
              ValInputIt                 values_first,
              KeyOutputIt                keys_output,
              ValOutputIt                values_output,
              BinaryPred                 binary_pred,
              BinaryOp                   binary_op)
{
    // struct workaround is required for HIP-clang
    struct workaround
    {
        THRUST_HOST static pair<KeyOutputIt, ValOutputIt> par(execution_policy<Derived>& policy,
                                                           KeyInputIt                 keys_first,
                                                           KeyInputIt                 keys_last,
                                                           ValInputIt                 values_first,
                                                           KeyOutputIt                keys_output,
                                                           ValOutputIt                values_output,
                                                           BinaryPred                 binary_pred,
                                                           BinaryOp                   binary_op)
        {
            return __reduce_by_key::reduce_by_key(policy,
                                                  keys_first,
                                                  keys_last,
                                                  values_first,
                                                  keys_output,
                                                  values_output,
                                                  binary_pred,
                                                  binary_op);
        }
        THRUST_DEVICE static pair<KeyOutputIt, ValOutputIt> seq(execution_policy<Derived>& policy,
                                                             KeyInputIt                 keys_first,
                                                             KeyInputIt                 keys_last,
                                                             ValInputIt  values_first,
                                                             KeyOutputIt keys_output,
                                                             ValOutputIt values_output,
                                                             BinaryPred  binary_pred,
                                                             BinaryOp    binary_op)
        {
            return thrust::reduce_by_key(cvt_to_seq(derived_cast(policy)),
                                         keys_first,
                                         keys_last,
                                         values_first,
                                         keys_output,
                                         values_output,
                                         binary_pred,
                                         binary_op);
        }
  };
  #if __THRUST_HAS_HIPRT__
    return workaround::par(policy,  keys_first, keys_last, values_first, keys_output, values_output, binary_pred, binary_op);
  #else
    return workaround::seq(policy,  keys_first, keys_last, values_first, keys_output, values_output, binary_pred, binary_op);
  #endif

}

THRUST_EXEC_CHECK_DISABLE template <class Derived,
                                        class KeyInputIt,
                                        class ValInputIt,
                                        class KeyOutputIt,
                                        class ValOutputIt,
                                        class BinaryPred>
THRUST_HIP_FUNCTION pair<KeyOutputIt, ValOutputIt>
reduce_by_key(execution_policy<Derived>& policy,
              KeyInputIt                 keys_first,
              KeyInputIt                 keys_last,
              ValInputIt                 values_first,
              KeyOutputIt                keys_output,
              ValOutputIt                values_output,
              BinaryPred                 binary_pred)
{
    using value_type = typename thrust::detail::eval_if<thrust::detail::is_output_iterator<ValOutputIt>::value,
                                                        thrust::iterator_value<ValInputIt>,
                                                        thrust::iterator_value<ValOutputIt>>::type;

    return hip_rocprim::reduce_by_key(policy,
                                      keys_first,
                                      keys_last,
                                      values_first,
                                      keys_output,
                                      values_output,
                                      binary_pred,
                                      plus<value_type>());
}

THRUST_EXEC_CHECK_DISABLE template <class Derived,
                                        class KeyInputIt,
                                        class ValInputIt,
                                        class KeyOutputIt,
                                        class ValOutputIt>
THRUST_HIP_FUNCTION pair<KeyOutputIt, ValOutputIt>
reduce_by_key(execution_policy<Derived>& policy,
              KeyInputIt                 keys_first,
              KeyInputIt                 keys_last,
              ValInputIt                 values_first,
              KeyOutputIt                keys_output,
              ValOutputIt                values_output)
{
    using KeyT = typename thrust::iterator_value<KeyInputIt>::type;
    return hip_rocprim::reduce_by_key(policy,
                                      keys_first,
                                      keys_last,
                                      values_first,
                                      keys_output,
                                      values_output,
                                      equal_to<KeyT>());
}

} // namespace  hip_rocprim
THRUST_NAMESPACE_END

#endif

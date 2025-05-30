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

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP
#include <thrust/detail/config.h>
#include <thrust/detail/type_traits.h>
#include <thrust/detail/type_traits/iterator/is_output_iterator.h>
#include <thrust/detail/temporary_array.h>
#include <thrust/functional.h>

#include <thrust/system/hip/detail/execution_policy.h>
#include <thrust/system/hip/detail/par_to_seq.h>
#include <thrust/system/hip/detail/util.h>
#include <thrust/detail/mpl/math.h>
#include <thrust/detail/alignment.h>
#include <thrust/detail/minmax.h>
#include <thrust/distance.h>

#include <cstdint>

// rocprim include
#include <rocprim/rocprim.hpp>


THRUST_NAMESPACE_BEGIN
template <typename DerivedPolicy,
          typename InputIterator,
          typename OutputIterator,
          typename AssociativeOperator>
THRUST_HIP_FUNCTION OutputIterator
inclusive_scan(const thrust::detail::execution_policy_base<DerivedPolicy>& exec,
               InputIterator                                               first,
               InputIterator                                               last,
               OutputIterator                                              result,
               AssociativeOperator                                         binary_op);

template <typename DerivedPolicy,
          typename InputIterator,
          typename OutputIterator,
          typename T,
          typename AssociativeOperator>
THRUST_HIP_FUNCTION OutputIterator
exclusive_scan(const thrust::detail::execution_policy_base<DerivedPolicy>& exec,
               InputIterator                                               first,
               InputIterator                                               last,
               OutputIterator                                              result,
               T                                                           init,
               AssociativeOperator                                         binary_op);

namespace hip_rocprim
{

namespace __scan
{
    template <typename Derived, typename InputIt, typename OutputIt, typename Size, typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_inclusive_scan(execution_policy<Derived>& policy,
                                                           void*             temporary_storage,
                                                           size_t&           storage_size,
                                                           InputIt           input,
                                                           OutputIt          output,
                                                           const Size        num_items,
                                                           ScanOp            scan_op,
                                                           const hipStream_t stream,
                                                           bool              debug_sync)
        -> std::enable_if_t<decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::inclusive_scan(
            temporary_storage, storage_size, input, output, num_items, scan_op, stream, debug_sync);
    }

    template <typename Derived, typename InputIt, typename OutputIt, typename Size, typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_inclusive_scan(execution_policy<Derived>& policy,
                                                           void*             temporary_storage,
                                                           size_t&           storage_size,
                                                           InputIt           input,
                                                           OutputIt          output,
                                                           const Size        num_items,
                                                           ScanOp            scan_op,
                                                           const hipStream_t stream,
                                                           bool              debug_sync)
        -> std::enable_if_t<!decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::deterministic_inclusive_scan(
            temporary_storage, storage_size, input, output, num_items, scan_op, stream, debug_sync);
    }

    template <typename Derived, typename InputIt, typename OutputIt, typename Size, typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION OutputIt
    inclusive_scan(execution_policy<Derived>& policy,
                   InputIt                    input_it,
                   OutputIt                   output_it,
                   Size                       num_items,
                   ScanOp                     scan_op)
    {
        if(num_items == 0)
            return output_it;

        size_t      storage_size = 0;
        hipStream_t stream       = hip_rocprim::stream(policy);
        bool        debug_sync   = THRUST_HIP_DEBUG_SYNC_FLAG;

        // Determine temporary device storage requirements.
        hip_rocprim::throw_on_error(invoke_inclusive_scan(policy,
                                                          nullptr,
                                                          storage_size,
                                                          input_it,
                                                          output_it,
                                                          num_items,
                                                          scan_op,
                                                          stream,
                                                          debug_sync),
                                    "scan failed on 1st step");

        // Allocate temporary storage.
        thrust::detail::temporary_array<std::uint8_t, Derived>
            tmp(policy, storage_size);
        void *ptr = static_cast<void*>(tmp.data().get());

        // Run scan.
        hip_rocprim::throw_on_error(invoke_inclusive_scan(policy,
                                                          ptr,
                                                          storage_size,
                                                          input_it,
                                                          output_it,
                                                          num_items,
                                                          scan_op,
                                                          stream,
                                                          debug_sync),
                                    "scan failed on 2nd step");

        hip_rocprim::throw_on_error(
            hip_rocprim::synchronize_optional(policy),
            "inclusive_scan: failed to synchronize"
        );

        return output_it + num_items;
    }

    template <typename Derived,
              typename InputIt,
              typename OutputIt,
              typename T,
              typename Size,
              typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_exclusive_scan(execution_policy<Derived>& policy,
                                                           void*             temporary_storage,
                                                           size_t&           storage_size,
                                                           InputIt           input,
                                                           OutputIt          output,
                                                           T                 init,
                                                           const Size        num_items,
                                                           ScanOp            scan_op,
                                                           const hipStream_t stream,
                                                           bool              debug_sync)
        -> std::enable_if_t<decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::exclusive_scan(temporary_storage,
                                       storage_size,
                                       input,
                                       output,
                                       init,
                                       num_items,
                                       scan_op,
                                       stream,
                                       debug_sync);
    }

    template <typename Derived,
              typename InputIt,
              typename OutputIt,
              typename T,
              typename Size,
              typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION auto invoke_exclusive_scan(execution_policy<Derived>& policy,
                                                           void*             temporary_storage,
                                                           size_t&           storage_size,
                                                           InputIt           input,
                                                           OutputIt          output,
                                                           T                 init,
                                                           const Size        num_items,
                                                           ScanOp            scan_op,
                                                           const hipStream_t stream,
                                                           bool              debug_sync)
        -> std::enable_if_t<!decltype(nondeterministic(policy))::value, hipError_t>
    {
        return rocprim::deterministic_exclusive_scan(temporary_storage,
                                                     storage_size,
                                                     input,
                                                     output,
                                                     init,
                                                     num_items,
                                                     scan_op,
                                                     stream,
                                                     debug_sync);
    }

    template <typename Derived, typename InputIt, typename OutputIt, typename Size, typename T, typename ScanOp>
    THRUST_HIP_RUNTIME_FUNCTION OutputIt
    exclusive_scan(execution_policy<Derived>& policy,
                   InputIt                    input_it,
                   OutputIt                   output_it,
                   Size                       num_items,
                   T                          init,
                   ScanOp                     scan_op)
    {
        if(num_items == 0)
            return output_it;

        size_t      storage_size = 0;
        hipStream_t stream       = hip_rocprim::stream(policy);
        bool        debug_sync   = THRUST_HIP_DEBUG_SYNC_FLAG;

        // Determine temporary device storage requirements.
        hip_rocprim::throw_on_error(invoke_exclusive_scan(policy,
                                                          nullptr,
                                                          storage_size,
                                                          input_it,
                                                          output_it,
                                                          init,
                                                          num_items,
                                                          scan_op,
                                                          stream,
                                                          debug_sync),
                                    "scan failed on 1st step");

        // Allocate temporary storage.
        thrust::detail::temporary_array<std::uint8_t, Derived>
            tmp(policy, storage_size);
        void *ptr = static_cast<void*>(tmp.data().get());

        // Run scan.
        hip_rocprim::throw_on_error(invoke_exclusive_scan(policy,
                                                          ptr,
                                                          storage_size,
                                                          input_it,
                                                          output_it,
                                                          init,
                                                          num_items,
                                                          scan_op,
                                                          stream,
                                                          debug_sync),
                                    "scan failed on 2nd step");

        hip_rocprim::throw_on_error(
            hip_rocprim::synchronize_optional(policy),
            "exclusive_scan: failed to synchronize"
        );

        return output_it + num_items;
    }

} // namespace __scan

//-------------------------
// Thrust API entry points
//-------------------------

template <class Derived, class InputIt, class Size, class OutputIt, class ScanOp>
THRUST_HIP_FUNCTION OutputIt
inclusive_scan_n(execution_policy<Derived>& policy,
                 InputIt                    input_it,
                 Size                       num_items,
                 OutputIt                   result,
                 ScanOp                     scan_op)
{
    // struct workaround is required for HIP-clang
    struct workaround
    {
        THRUST_HOST static OutputIt par(execution_policy<Derived>& policy,
                                     InputIt                    input_it,
                                     Size                       num_items,
                                     OutputIt                   result,
                                     ScanOp                     scan_op)
        {
            return __scan::inclusive_scan(policy, input_it, result, num_items, scan_op);
        }
        THRUST_DEVICE static OutputIt seq(execution_policy<Derived>& policy,
                                       InputIt                    input_it,
                                       Size                       num_items,
                                       OutputIt                   result,
                                       ScanOp                     scan_op)
        {
            return thrust::inclusive_scan(
                cvt_to_seq(derived_cast(policy)), input_it, input_it + num_items, result, scan_op);
        }
  };
  #if __THRUST_HAS_HIPRT__
    return workaround::par(policy, input_it, num_items, result, scan_op);
  #else
    return workaround::seq(policy, input_it, num_items, result, scan_op);
  #endif
}

template <class Derived, class InputIt, class OutputIt, class ScanOp>
OutputIt THRUST_HIP_FUNCTION
inclusive_scan(execution_policy<Derived>& policy,
               InputIt                    first,
               InputIt                    last,
               OutputIt                   result,
               ScanOp                     scan_op)
{
    using diff_t = typename thrust::iterator_traits<InputIt>::difference_type;
    diff_t const num_items = thrust::distance(first, last);
    return thrust::hip_rocprim::inclusive_scan_n(policy,
                                                 first,
                                                 num_items,
                                                 result,
                                                 scan_op);
}

template <class Derived, class InputIt, class OutputIt>
OutputIt THRUST_HIP_FUNCTION
inclusive_scan(execution_policy<Derived>& policy,
               InputIt                    first,
               OutputIt                   last,
               OutputIt                   result)
{
    return thrust::hip_rocprim::inclusive_scan(policy,
                                               first,
                                               last,
                                               result,
                                               thrust::plus<>{});
}

THRUST_EXEC_CHECK_DISABLE template <class Derived,
                                        class InputIt,
                                        class Size,
                                        class OutputIt,
                                        class T,
                                        class ScanOp>
OutputIt THRUST_HIP_FUNCTION exclusive_scan_n(execution_policy<Derived>& policy,
                                              InputIt                    first,
                                              Size                       num_items,
                                              OutputIt                   result,
                                              T                          init,
                                              ScanOp                     scan_op)
{
    // struct workaround is required for HIP-clang
    struct workaround
    {
        THRUST_HOST static OutputIt par(execution_policy<Derived>& policy,
                                     InputIt                    first,
                                     Size                       num_items,
                                     OutputIt                   result,
                                     T                          init,
                                     ScanOp                     scan_op)
        {
            return __scan::exclusive_scan(policy, first, result, num_items, init, scan_op);
        }

        THRUST_DEVICE static OutputIt seq(execution_policy<Derived>& policy,
                                       InputIt                    first,
                                       Size                       num_items,
                                       OutputIt                   result,
                                       T                          init,
                                       ScanOp                     scan_op)
        {
            return thrust::exclusive_scan(
                cvt_to_seq(derived_cast(policy)), first, first + num_items, result, init, scan_op);
        }
  };

  #if __THRUST_HAS_HIPRT__
    return workaround::par(policy, first, num_items, result, init, scan_op);
  #else
    return workaround::seq(policy, first, num_items, result, init, scan_op);
  #endif
}

template <class Derived, class InputIt, class OutputIt, class T, class ScanOp>
OutputIt THRUST_HIP_FUNCTION
exclusive_scan(execution_policy<Derived>& policy,
               InputIt                    first,
               InputIt                    last,
               OutputIt                   result,
               T                          init,
               ScanOp                     scan_op)
{
    using diff_t = typename thrust::iterator_traits<InputIt>::difference_type;
    diff_t const num_items = thrust::distance(first, last);
    return hip_rocprim::exclusive_scan_n(policy,
                                         first,
                                         num_items,
                                         result,
                                         init,
                                         scan_op);
}

template <class Derived, class InputIt, class OutputIt, class T>
OutputIt THRUST_HIP_FUNCTION
exclusive_scan(execution_policy<Derived>& policy,
               InputIt                    first,
               OutputIt                   last,
               OutputIt                   result,
               T                          init)
{
    return thrust::hip_rocprim::exclusive_scan(policy,
                                               first,
                                               last,
                                               result,
                                               init,
                                               thrust::plus<>{});
}

template <class Derived, class InputIt, class OutputIt>
OutputIt THRUST_HIP_FUNCTION
exclusive_scan(execution_policy<Derived>& policy,
               InputIt                    first,
               OutputIt                   last,
               OutputIt                   result)
{
    using init_type = typename thrust::iterator_traits<InputIt>::value_type;
    return hip_rocprim::exclusive_scan(policy, first, last, result, init_type{});
}

} // namespace  hip_rocprim

THRUST_NAMESPACE_END

#include <thrust/scan.h>

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP

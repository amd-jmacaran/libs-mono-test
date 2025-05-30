/******************************************************************************
 * Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright© 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
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

// TODO: Optimize for thrust::plus

// TODO: Move into system::hip

#pragma once

#include <thrust/detail/config.h>

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP

#include <thrust/system/hip/config.h>

#include <thrust/system/hip/detail/async/customization.h>
#include <thrust/system/hip/detail/reduce.h>
#include <thrust/system/hip/future.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/distance.h>

#include <type_traits>

// rocprim include
#include <rocprim/rocprim.hpp>

THRUST_NAMESPACE_BEGIN

namespace system { namespace hip { namespace detail
{

template <
  typename DerivedPolicy
, typename ForwardIt, typename Size, typename T, typename BinaryOp
>
auto async_reduce_n(
  execution_policy<DerivedPolicy>& policy,
  ForwardIt                        first,
  Size                             n,
  T                                init,
  BinaryOp                         op
) -> unique_eager_future<remove_cvref_t<T>>
{
  using U = remove_cvref_t<T>;
  auto const device_alloc = get_async_device_allocator(policy);

  using pointer
    = typename thrust::detail::allocator_traits<decltype(device_alloc)>::
      template rebind_traits<U>::pointer;

  unique_eager_future_promise_pair<U, pointer> fp;

  // Determine temporary device storage requirements.

  size_t tmp_size = 0;
  thrust::hip_rocprim::throw_on_error(
    rocprim::reduce(
      nullptr
    , tmp_size
    , first
    , static_cast<U*>(nullptr)
    , init
    , n
    , op
    , nullptr // Null stream, just for sizing.
    , THRUST_HIP_DEBUG_SYNC_FLAG
    )
  , "after reduction sizing"
  );

  // Allocate temporary storage.

  auto content = uninitialized_allocate_unique_n<std::uint8_t>(
    device_alloc, sizeof(U) + tmp_size
  );

  // The array was dynamically allocated, so we assume that it's suitably
  // aligned for any type of data. `malloc`/`hipMalloc`/`new`/`std::allocator`
  // make this guarantee.
  auto const content_ptr = content.get();
  U* const ret_ptr = thrust::detail::aligned_reinterpret_cast<T*>(
    raw_pointer_cast(content_ptr)
  );
  void* const tmp_ptr = static_cast<void*>(
    raw_pointer_cast(content_ptr + sizeof(U))
  );

  // Set up stream with dependencies.

  hipStream_t user_raw_stream = thrust::hip_rocprim::stream(policy);

  if (thrust::hip_rocprim::default_stream() != user_raw_stream)
  {
    fp = make_dependent_future<U, pointer>(
      [] (decltype(content) const& c)
      {
        return pointer(
          thrust::detail::aligned_reinterpret_cast<U*>(
            raw_pointer_cast(c.get())
          )
        );
      }
    , std::tuple_cat(
        std::make_tuple(
          std::move(content)
        , unique_stream(nonowning, user_raw_stream)
        )
      , extract_dependencies(
          std::move(thrust::detail::derived_cast(policy))
        )
      )
    );
  }
  else
  {
    fp = make_dependent_future<U, pointer>(
      [] (decltype(content) const& c)
      {
        return pointer(
          thrust::detail::aligned_reinterpret_cast<U*>(
            raw_pointer_cast(c.get())
          )
        );
      }
    , std::tuple_cat(
        std::make_tuple(
          std::move(content)
        )
      , extract_dependencies(
          std::move(thrust::detail::derived_cast(policy))
        )
      )
    );
  }

  // Run reduction.

  thrust::hip_rocprim::throw_on_error(
    rocprim::reduce(
      tmp_ptr
    , tmp_size
    , first
    , ret_ptr
    , init
    , n
    , op
    , fp.future.stream().native_handle()
    , THRUST_HIP_DEBUG_SYNC_FLAG
    )
  , "after reduction launch"
  );

  return std::move(fp.future);
}

}}} // namespace system::hip::detail

namespace hip_rocprim
{

// ADL entry point.
template <
  typename DerivedPolicy
, typename ForwardIt, typename Sentinel, typename T, typename BinaryOp
>
auto async_reduce(
  execution_policy<DerivedPolicy>& policy,
  ForwardIt                        first,
  Sentinel                         last,
  T                                init,
  BinaryOp                         op
)
THRUST_RETURNS(
  thrust::system::hip::detail::async_reduce_n(
    policy, first, distance(first, last), init, op
  )
)

} // hip_rocprim
///////////////////////////////////////////////////////////////////////////////

namespace system { namespace hip { namespace detail
{

template <
  typename DerivedPolicy
, typename ForwardIt, typename Size, typename OutputIt
, typename T, typename BinaryOp
>
auto async_reduce_into_n(
  execution_policy<DerivedPolicy>& policy
, ForwardIt                        first
, Size                             n
, OutputIt                         output
, T                                init
, BinaryOp                         op
) -> unique_eager_event
{
  using U = remove_cvref_t<T>;

  auto const device_alloc = get_async_device_allocator(policy);

  unique_eager_event e;

  // Determine temporary device storage requirements.

  size_t tmp_size = 0;
  thrust::hip_rocprim::throw_on_error(
    rocprim::reduce(
      nullptr
    , tmp_size
    , first
    , static_cast<U*>(nullptr)
    , init
    , n
    , op
    , nullptr // Null stream, just for sizing.
    , THRUST_HIP_DEBUG_SYNC_FLAG
    )
  , "after reduction sizing"
  );

  // Allocate temporary storage.

  auto content = uninitialized_allocate_unique_n<std::uint8_t>(
    device_alloc, tmp_size
  );

  // The array was dynamically allocated, so we assume that it's suitably
  // aligned for any type of data. `malloc`/`hipMalloc`/`new`/`std::allocator`
  // make this guarantee.
  auto const content_ptr = content.get();

  void* const tmp_ptr = static_cast<void*>(
    raw_pointer_cast(content_ptr)
  );

  // Set up stream with dependencies.

  hipStream_t const user_raw_stream = thrust::hip_rocprim::stream(policy);

  if (thrust::hip_rocprim::default_stream() != user_raw_stream)
  {
    e = make_dependent_event(
      std::tuple_cat(
        std::make_tuple(
          std::move(content)
        , unique_stream(nonowning, user_raw_stream)
        )
      , extract_dependencies(
          std::move(thrust::detail::derived_cast(policy))
        )
      )
    );
  }
  else
  {
    e = make_dependent_event(
      std::tuple_cat(
        std::make_tuple(
          std::move(content)
        )
      , extract_dependencies(
          std::move(thrust::detail::derived_cast(policy))
        )
      )
    );
  }

  // Run reduction.

  thrust::hip_rocprim::throw_on_error(
    rocprim::reduce(
      tmp_ptr
    , tmp_size
    , first
    , output
    , init
    , n
    , op
    , e.stream().native_handle()
    , THRUST_HIP_DEBUG_SYNC_FLAG
    )
  , "after reduction launch"
  );

  return e;
}

}}} // namespace system::hip::detail

namespace hip_rocprim
{

// ADL entry point.
template <
  typename DerivedPolicy
, typename ForwardIt, typename Sentinel, typename OutputIt
, typename T, typename BinaryOp
>
auto async_reduce_into(
  execution_policy<DerivedPolicy>& policy
, ForwardIt                        first
, Sentinel                         last
, OutputIt                         output
, T                                init
, BinaryOp                         op
)
THRUST_RETURNS(
  thrust::system::hip::detail::async_reduce_into_n(
    policy, first, distance(first, last), output, init, op
  )
)

} // hip_rocprim

THRUST_NAMESPACE_END

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HIP

/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <thrust/detail/config.h>
#include <thrust/system/detail/generic/equal.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/detail/internal_functional.h>
#include <thrust/mismatch.h>

#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA
#include <cuda/std/functional>
#endif

THRUST_NAMESPACE_BEGIN
namespace system
{
namespace detail
{
namespace generic
{


template<typename DerivedPolicy, typename InputIterator1, typename InputIterator2>
THRUST_HOST_DEVICE
bool equal(thrust::execution_policy<DerivedPolicy> &exec, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2)
{
#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA
  using namespace ::cuda::std;
#else // THRUST_DEVICE_SYSTEM != THRUST_DEVICE_SYSTEM_CUDA
  using namespace std;
#endif // THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA
  return thrust::equal(exec, first1, last1, first2, equal_to<>());
}

// the == below could be a __host__ function in the case of std::vector::iterator::operator==
// we make this exception for equal and use nv_exec_check_disable because it is used in vector's implementation
THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy, typename InputIterator1, typename InputIterator2, typename BinaryPredicate>
THRUST_HOST_DEVICE
bool equal(thrust::execution_policy<DerivedPolicy> &exec, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicate binary_pred)
{
  return thrust::mismatch(exec, first1, last1, first2, binary_pred).first == last1;
}


} // end generic
} // end detail
} // end system
THRUST_NAMESPACE_END


/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *  Modifications Copyright© 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
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

/*! \file trivial_copy.h
 *  \brief Sequential copy algorithms for plain-old-data.
 */

#pragma once

#include <thrust/detail/config.h>
#include <cstring>
#include <thrust/system/detail/sequential/general_copy.h>

#include <thrust/detail/nv_target.h>

THRUST_NAMESPACE_BEGIN
namespace system
{
namespace detail
{
namespace sequential
{


template<typename T>
THRUST_HOST_DEVICE
  T *trivial_copy_n(const T *first,
                    std::ptrdiff_t n,
                    T *result)
{
  if(n == 0)
  {
    // If `first` or `result` is an invalid pointer,
    // the behavior of `std::memmove` is undefined, even if `n` is zero.
    return result;
  }

  T* return_value = nullptr;

  NV_IF_TARGET(NV_IS_HOST, (
    std::memmove(result, first, n * sizeof(T));
    return_value = result + n;
  ), ( // NV_IS_DEVICE:
    return_value = thrust::system::detail::sequential::general_copy_n(first, n, result);
  ));
  return return_value;
} // end trivial_copy_n()


} // end namespace sequential
} // end namespace detail
} // end namespace system
THRUST_NAMESPACE_END

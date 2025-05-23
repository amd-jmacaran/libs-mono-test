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

#include <thrust/advance.h>
#include <thrust/detail/config.h>
#include <thrust/system/detail/generic/distance.h>
#include <thrust/iterator/iterator_traits.h>

THRUST_NAMESPACE_BEGIN

THRUST_EXEC_CHECK_DISABLE
template<typename InputIterator>
inline THRUST_HOST_DEVICE
  typename thrust::iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last)
{
  return thrust::system::detail::generic::distance(first, last);
} // end distance()

THRUST_NAMESPACE_END

/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *  Copyright 2013 Filipe RNC Maia
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

#pragma once

#include <thrust/detail/config.h>

#include <thrust/complex.h>
#include <thrust/detail/type_traits.h>

#include <cmath>
#include <type_traits>

THRUST_NAMESPACE_BEGIN

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
pow(const complex<T0>& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return exp(log(complex<T>(x)) * complex<T>(y));
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type> pow(const complex<T0>& x, const T1& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return exp(log(complex<T>(x)) * T(y));
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type> pow(const T0& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  #ifdef __HIP_DEVICE_COMPILE__
    using ::log;
  #else
    // Find `log` by ADL.
    using std::log;
  #endif
  return exp(log(T(x)) * complex<T>(y));
}

THRUST_NAMESPACE_END

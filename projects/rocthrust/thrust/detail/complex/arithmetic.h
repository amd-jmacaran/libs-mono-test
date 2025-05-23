/*
 *  Copyright 2008-2021 NVIDIA Corporation
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
#include <thrust/detail/complex/c99math.h>
#include <cfloat>
#include <cmath>

THRUST_NAMESPACE_BEGIN

  /* --- Binary Arithmetic Operators --- */

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator+(const complex<T0>& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() + y.real(), x.imag() + y.imag());
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator+(const complex<T0>& x, const T1& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() + y, x.imag());
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator+(const T0& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x + y.real(), y.imag());
}


template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator-(const complex<T0>& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() - y.real(), x.imag() - y.imag());
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator-(const complex<T0>& x, const T1& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() - y, x.imag());
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator-(const T0& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x - y.real(), -y.imag());
}


template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator*(const complex<T0>& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>( x.real() * y.real() - x.imag() * y.imag()
			             , x.real() * y.imag() + x.imag() * y.real());
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator*(const complex<T0>& x, const T1& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() * y, x.imag() * y);
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator*(const T0& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x * y.real(), x * y.imag());
}


template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator/(const complex<T0>& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;

  // Find `abs` by ADL.
  using std::abs;

  T s = abs(y.real()) + abs(y.imag());

  T oos = T(1.0) / s;

  T ars = x.real() * oos;
  T ais = x.imag() * oos;
  T brs = y.real() * oos;
  T bis = y.imag() * oos;

  s = (brs * brs) + (bis * bis);

  oos = T(1.0) / s;

  complex<T> quot( ((ars * brs) + (ais * bis)) * oos
                 , ((ais * brs) - (ars * bis)) * oos);
  return quot;
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator/(const complex<T0>& x, const T1& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x.real() / y, x.imag() / y);
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type>
operator/(const T0& x, const complex<T1>& y)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;
  return complex<T>(x) / y;
}



/* --- Unary Arithmetic Operators --- */

template <typename T>
THRUST_HOST_DEVICE complex<T> operator+(const complex<T>& y)
{
  return y;
}

template <typename T>
THRUST_HOST_DEVICE complex<T> operator-(const complex<T>& y)
{
  return y * -T(1);
}


/* --- Other Basic Arithmetic Functions --- */

// As std::hypot is only C++11 we have to use the C interface
template <typename T>
THRUST_HOST_DEVICE T abs(const complex<T>& z)
{
  return hypot(z.real(), z.imag());
}

// XXX Why are we specializing here?
namespace detail {
namespace complex {

THRUST_HOST_DEVICE inline float abs(const thrust::complex<float>& z)
{
  return hypotf(z.real(),z.imag());
}

THRUST_HOST_DEVICE inline double abs(const thrust::complex<double>& z)
{
  return hypot(z.real(),z.imag());
}

} // end namespace complex
} // end namespace detail

template <>
THRUST_HOST_DEVICE inline float abs(const complex<float>& z)
{
  return detail::complex::abs(z);
}

template <>
THRUST_HOST_DEVICE inline double abs(const complex<double>& z)
{
  return detail::complex::abs(z);
}


template <typename T>
THRUST_HOST_DEVICE T arg(const complex<T>& z)
{
  // Find `atan2` by ADL.
  #ifdef __HIP_DEVICE_COMPILE__
    using ::atan2;
  #else
    using std::atan2;
  #endif
  return atan2(z.imag(), z.real());
}


template <typename T>
THRUST_HOST_DEVICE complex<T> conj(const complex<T>& z)
{
  return complex<T>(z.real(), -z.imag());
}


template <typename T>
THRUST_HOST_DEVICE T norm(const complex<T>& z)
{
  return z.real() * z.real() + z.imag() * z.imag();
}

// XXX Why specialize these, we could just rely on ADL.
template <>
THRUST_HOST_DEVICE inline float norm(const complex<float>& z)
{
  // Find `abs` and `sqrt` by ADL.
  using std::abs;
  #ifdef __HIP_DEVICE_COMPILE__
    using ::sqrt;
  #else
    using std::sqrt;
  #endif

  if (abs(z.real()) < sqrt(FLT_MIN) && abs(z.imag()) < sqrt(FLT_MIN))
  {
    float a = z.real() * 4.0f;
    float b = z.imag() * 4.0f;
    return (a * a + b * b) / 16.0f;
  }

  return z.real() * z.real() + z.imag() * z.imag();
}

template <>
THRUST_HOST_DEVICE inline double norm(const complex<double>& z)
{
  // Find `abs` and `sqrt` by ADL.
  using std::abs;
  #ifdef __HIP_DEVICE_COMPILE__
    using ::sqrt;
  #else
    using std::sqrt;
  #endif


  if (abs(z.real()) < sqrt(DBL_MIN) && abs(z.imag()) < sqrt(DBL_MIN))
  {
    double a = z.real() * 4.0;
    double b = z.imag() * 4.0;
    return (a * a + b * b) / 16.0;
  }

  return z.real() * z.real() + z.imag() * z.imag();
}


template <typename T0, typename T1>
THRUST_HOST_DEVICE complex<typename detail::promoted_numerical_type<T0, T1>::type> polar(const T0& m, const T1& theta)
{
  using T = typename detail::promoted_numerical_type<T0, T1>::type;

  // Find `cos` and `sin` by ADL.
  #ifdef __HIP_DEVICE_COMPILE__
    using ::cos;
    using ::sin;
  #else
    using std::cos;
    using std::sin;
  #endif

  return complex<T>(m * cos(theta), m * sin(theta));
}

THRUST_NAMESPACE_END

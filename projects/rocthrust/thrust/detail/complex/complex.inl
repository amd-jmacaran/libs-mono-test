/*
 *  Copyright 2008-2021 NVIDIA Corporation
 *  Copyright 2013 Filipe RNC Maia
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
#include <thrust/type_traits/is_trivially_relocatable.h>

THRUST_NAMESPACE_BEGIN

/* --- Constructors --- */

template <typename T>
THRUST_HOST_DEVICE complex<T>::complex(const T& re)
  // Initialize the storage in the member initializer list using C++ unicorn
  // initialization. This allows `complex<T const>` to work.
  : data{re, T()}
{}


template <typename T>
THRUST_HOST_DEVICE complex<T>::complex(const T& re, const T& im)
  // Initialize the storage in the member initializer list using C++ unicorn
  // initialization. This allows `complex<T const>` to work.
  : data{re, im}
{}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>::complex(const complex<U>& z)
  // Initialize the storage in the member initializer list using C++ unicorn
  // initialization. This allows `complex<T const>` to work.
  // We do a functional-style cast here to suppress conversion warnings.
  : data{T(z.real()), T(z.imag())}
{}

template <typename T>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE complex<T>::complex(const std::complex<T>& z)
  // Initialize the storage in the member initializer list using C++ unicorn
  // initialization. This allows `complex<T const>` to work.
  : data{THRUST_STD_COMPLEX_REAL(z), THRUST_STD_COMPLEX_IMAG(z)}
{}

template <typename T>
template <typename U>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE complex<T>::complex(const std::complex<U>& z)
  // Initialize the storage in the member initializer list using C++ unicorn
  // initialization. This allows `complex<T const>` to work.
  // We do a functional-style cast here to suppress conversion warnings.
  : data{T(THRUST_STD_COMPLEX_REAL(z)), T(THRUST_STD_COMPLEX_IMAG(z))}
{}



/* --- Assignment Operators --- */

template <typename T>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator=(const T& re)
{
  real(re);
  imag(T());
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator=(const complex<U>& z)
{
  real(T(z.real()));
  imag(T(z.imag()));
  return *this;
}

template <typename T>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE complex<T>& complex<T>::operator=(const std::complex<T>& z)
{
  real(THRUST_STD_COMPLEX_REAL(z));
  imag(THRUST_STD_COMPLEX_IMAG(z));
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE complex<T>& complex<T>::operator=(const std::complex<U>& z)
{
  real(T(THRUST_STD_COMPLEX_REAL(z)));
  imag(T(THRUST_STD_COMPLEX_IMAG(z)));
  return *this;
}



/* --- Compound Assignment Operators --- */

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator+=(const complex<U>& z)
{
  *this = *this + z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator-=(const complex<U>& z)
{
  *this = *this - z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator*=(const complex<U>& z)
{
  *this = *this * z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator/=(const complex<U>& z)
{
  *this = *this / z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator+=(const U& z)
{
  *this = *this + z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator-=(const U& z)
{
  *this = *this - z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator*=(const U& z)
{
  *this = *this * z;
  return *this;
}

template <typename T>
template <typename U>
THRUST_HOST_DEVICE complex<T>& complex<T>::operator/=(const U& z)
{
  *this = *this / z;
  return *this;
}



/* --- Equality Operators --- */

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator==(const complex<T0>& x, const complex<T1>& y)
{
  return x.real() == y.real() && x.imag() == y.imag();
}

template <typename T0, typename T1>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE bool operator==(const complex<T0>& x, const std::complex<T1>& y)
{
  return x.real() == THRUST_STD_COMPLEX_REAL(y) && x.imag() == THRUST_STD_COMPLEX_IMAG(y);
}

template <typename T0, typename T1>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE bool operator==(const std::complex<T0>& x, const complex<T1>& y)
{
  return THRUST_STD_COMPLEX_REAL(x) == y.real() && THRUST_STD_COMPLEX_IMAG(x) == y.imag();
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator==(const T0& x, const complex<T1>& y)
{
  return x == y.real() && y.imag() == T1();
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator==(const complex<T0>& x, const T1& y)
{
  return x.real() == y && x.imag() == T1();
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator!=(const complex<T0>& x, const complex<T1>& y)
{
  return !(x == y);
}

template <typename T0, typename T1>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE bool operator!=(const complex<T0>& x, const std::complex<T1>& y)
{
  return !(x == y);
}

template <typename T0, typename T1>
THRUST_HOST THRUST_STD_COMPLEX_DEVICE bool operator!=(const std::complex<T0>& x, const complex<T1>& y)
{
  return !(x == y);
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator!=(const T0& x, const complex<T1>& y)
{
  return !(x == y);
}

template <typename T0, typename T1>
THRUST_HOST_DEVICE bool operator!=(const complex<T0>& x, const T1& y)
{
  return !(x == y);
}

template <typename T>
struct proclaim_trivially_relocatable<complex<T> > : thrust::true_type {};

THRUST_NAMESPACE_END

#include <thrust/detail/complex/arithmetic.h>
#include <thrust/detail/complex/cproj.h>
#include <thrust/detail/complex/cexp.h>
#include <thrust/detail/complex/cexpf.h>
#include <thrust/detail/complex/clog.h>
#include <thrust/detail/complex/clogf.h>
#include <thrust/detail/complex/cpow.h>
#include <thrust/detail/complex/ccosh.h>
#include <thrust/detail/complex/ccoshf.h>
#include <thrust/detail/complex/csinh.h>
#include <thrust/detail/complex/csinhf.h>
#include <thrust/detail/complex/ctanh.h>
#include <thrust/detail/complex/ctanhf.h>
#include <thrust/detail/complex/csqrt.h>
#include <thrust/detail/complex/csqrtf.h>
#include <thrust/detail/complex/catrig.h>
#include <thrust/detail/complex/catrigf.h>
#include <thrust/detail/complex/stream.h>

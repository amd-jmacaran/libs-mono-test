/*
 *  Copyright 2008-2018 NVIDIA Corporation
 *  Modifications Copyright (c) 2024-2025, Advanced Micro Devices, Inc.  All rights reserved.
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
#include <thrust/detail/raw_reference_cast.h>
#include <thrust/detail/reference_forward_declaration.h>
#include <thrust/pair.h>
#include <thrust/tuple.h>

#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA
#include <cuda/std/tuple>
#include <cuda/std/type_traits>
#elif defined(__has_include)
#if __has_include(<cuda/std/tuple>)
#include <cuda/std/tuple>
#endif // __has_include(<cuda/std/tuple>)
#if __has_include(<cuda/std/type_traits>)
#include <cuda/std/type_traits>
#endif // __has_include(<cuda/std/type_traits>)
#endif // THRUST_DEVICE_SYSTEM

#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA

THRUST_NAMESPACE_BEGIN

namespace detail
{

template <typename... Ts >
class tuple_of_iterator_references;

template<class U, class T>
struct maybe_unwrap_nested {
  THRUST_HOST_DEVICE U operator()(const T& t) const {
    return t;
  }
};

template<class... Us, class... Ts>
struct maybe_unwrap_nested<thrust::tuple<Us...>, tuple_of_iterator_references<Ts...>> {
  THRUST_HOST_DEVICE thrust::tuple<Us...> operator()(const tuple_of_iterator_references<Ts...>& t) const {
    return t.template __to_tuple<Us...>(typename ::cuda::std::__make_tuple_indices<sizeof...(Ts)>::type{});
  }
};

template < typename... Ts >
class tuple_of_iterator_references : public thrust::tuple<Ts...>
{
public:
  using super_t = thrust::tuple<Ts...>;
  using super_t::super_t;

  tuple_of_iterator_references() = default;

  // allow implicit construction from tuple<refs>
  inline THRUST_HOST_DEVICE tuple_of_iterator_references(const super_t& other)
      : super_t(other)
  {}

  inline THRUST_HOST_DEVICE tuple_of_iterator_references(super_t&& other)
      : super_t(::cuda::std::move(other))
  {}

  // allow assignment from tuples
  // XXX might be worthwhile to guard this with an enable_if is_assignable
  THRUST_EXEC_CHECK_DISABLE
  template <typename... Us>
  inline THRUST_HOST_DEVICE tuple_of_iterator_references& operator=(const thrust::tuple<Us...>& other)
  {
    super_t::operator=(other);
    return *this;
  }

  // allow assignment from pairs
  // XXX might be worthwhile to guard this with an enable_if is_assignable
  THRUST_EXEC_CHECK_DISABLE
  template <typename U1, typename U2>
  inline THRUST_HOST_DEVICE tuple_of_iterator_references& operator=(const thrust::pair<U1, U2>& other)
  {
    super_t::operator=(other);
    return *this;
  }

  // allow assignment from reference<tuple>
  // XXX perhaps we should generalize to reference<T>
  //     we could captures reference<pair> this way
  THRUST_EXEC_CHECK_DISABLE
  template <typename Pointer, typename Derived, typename... Us>
  inline THRUST_HOST_DEVICE tuple_of_iterator_references&
  operator=(const thrust::reference<thrust::tuple<Us...>, Pointer, Derived>& other)
  {
    using tuple_type = thrust::tuple<Us...>;

    // XXX perhaps this could be accelerated
    super_t::operator=(tuple_type{other});
    return *this;
  }

  template <class... Us, ::cuda::std::__enable_if_t<sizeof...(Us) == sizeof...(Ts), int> = 0>
  inline THRUST_HOST_DEVICE constexpr operator thrust::tuple<Us...>() const
  {
    return __to_tuple<Us...>(typename ::cuda::std::__make_tuple_indices<sizeof...(Ts)>::type{});
  }

  // this overload of swap() permits swapping tuple_of_iterator_references returned as temporaries from
  // iterator dereferences
  template <class... Us>
  inline THRUST_HOST_DEVICE friend void swap(tuple_of_iterator_references&& x, tuple_of_iterator_references<Us...>&& y)
  {
    x.swap(y);
  }

  template <class... Us, size_t... Id>
  inline THRUST_HOST_DEVICE constexpr thrust::tuple<Us...> __to_tuple(::cuda::std::__tuple_indices<Id...>) const
  {
    return {maybe_unwrap_nested<Us, Ts>{}(get<Id>(*this))...};
  }
};

} // namespace detail

template<class... Ts>
struct __is_tuple_of_iterator_references<THRUST_NS_QUALIFIER::detail::tuple_of_iterator_references<Ts...>>
    : integral_constant<bool, true>
{};

THRUST_NAMESPACE_END

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// define tuple_size, tuple_element, etc.
template <class... Ts>
struct tuple_size<THRUST_NS_QUALIFIER::detail::tuple_of_iterator_references<Ts...>>
    : integral_constant<size_t, sizeof...(Ts)>
{};

template <size_t Id, class... Ts>
struct tuple_element<Id, THRUST_NS_QUALIFIER::detail::tuple_of_iterator_references<Ts...>>
    : _CUDA_VSTD::tuple_element<Id, _CUDA_VSTD::tuple<Ts...>>
{};

_LIBCUDACXX_END_NAMESPACE_STD

// structured bindings suppport
namespace std
{

template <class... Ts>
struct tuple_size<THRUST_NS_QUALIFIER::detail::tuple_of_iterator_references<Ts...>>
    : integral_constant<size_t, sizeof...(Ts)>
{};

template <size_t Id, class... Ts>
struct tuple_element<Id, THRUST_NS_QUALIFIER::detail::tuple_of_iterator_references<Ts...>>
    : _CUDA_VSTD::tuple_element<Id, _CUDA_VSTD::tuple<Ts...>>
{};

} // namespace std

#else // THRUST_DEVICE_SYSTEM != THRUST_DEVICE_SYSTEM_CUDA

THRUST_NAMESPACE_BEGIN

namespace detail
{

template<
  typename... Ts
>
  class tuple_of_iterator_references
    : public thrust::tuple<Ts...>
{
  private:
    using super_t = thrust::tuple<Ts...>;

  public:
    // allow implicit construction from tuple<refs>
    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references(const super_t &other)
      : super_t(other)
    {}

    // allow assignment from tuples
    // XXX might be worthwhile to guard this with an enable_if is_assignable
    THRUST_EXEC_CHECK_DISABLE
    template<typename... Us>
    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references &operator=(const thrust::tuple<Us...> &other)
    {
      super_t::operator=(other);
      return *this;
    }

    // allow assignment from pairs
    // XXX might be worthwhile to guard this with an enable_if is_assignable
    THRUST_EXEC_CHECK_DISABLE
    template<typename U1, typename U2>
    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references &operator=(const thrust::pair<U1,U2> &other)
    {
      super_t::operator=(other);
      return *this;
    }

    // allow assignment from reference<tuple>
    // XXX perhaps we should generalize to reference<T>
    //     we could captures reference<pair> this way
    THRUST_EXEC_CHECK_DISABLE
    template<typename Pointer, typename Derived,
             typename... Us>
    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references &
    operator=(const thrust::reference<thrust::tuple<Us...>, Pointer, Derived> &other)
    {
      using tuple_type = thrust::tuple<Us...>;

      // XXX perhaps this could be accelerated
      tuple_type other_tuple = other;
      super_t::operator=(other_tuple);
      return *this;
    }


    // duplicate thrust::tuple's constructors
    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references() {}

    inline THRUST_HOST_DEVICE
    tuple_of_iterator_references(typename access_traits<Ts>::parameter_type... ts)
      : super_t(ts...)
    {}
};


// this overload of swap() permits swapping tuple_of_iterator_references returned as temporaries from
// iterator dereferences
template<
  typename... Ts,
  typename... Us
>
inline THRUST_HOST_DEVICE
void swap(tuple_of_iterator_references<Ts...> x,
          tuple_of_iterator_references<Us...> y)
{
  x.swap(y);
}


} // end detail

// define tuple_size, tuple_element, etc.
template<class... Ts>
struct tuple_size<detail::tuple_of_iterator_references<Ts...>>
  : std::integral_constant<size_t, sizeof...(Ts)>
{};

template<size_t i>
struct tuple_element<i, detail::tuple_of_iterator_references<>> {};


template<class T, class... Ts>
struct tuple_element<0, detail::tuple_of_iterator_references<T,Ts...>>
{
  using type = T;
};


template<size_t i, class T, class... Ts>
struct tuple_element<i, detail::tuple_of_iterator_references<T,Ts...>>
{
  using type = typename tuple_element<i - 1, detail::tuple_of_iterator_references<Ts...>>::type;
};


THRUST_NAMESPACE_END

#endif // THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA

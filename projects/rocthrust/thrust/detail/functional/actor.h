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

// Portions of this code are derived from
//
// Manjunath Kudlur's Carbon library
//
// and
//
// Based on Boost.Phoenix v1.2
// Copyright (c) 2001-2002 Joel de Guzman

#pragma once

#include <thrust/detail/config.h>
#include <thrust/tuple.h>
#include <thrust/detail/functional/value.h>
#include <thrust/detail/functional/composite.h>
#include <thrust/detail/functional/operators/assignment_operator.h>
#include <thrust/detail/raw_reference_cast.h>
#include <thrust/detail/type_traits/result_of_adaptable_function.h>

THRUST_NAMESPACE_BEGIN
namespace detail
{
namespace functional
{

// eval_ref<T> is
// - T when T is a subclass of thrust::reference
// - T& otherwise
// This is used to let thrust::references pass through actor evaluations.
template <typename T>
using eval_ref = typename std::conditional<
  thrust::detail::is_wrapped_reference<T>::value, T, T&>::type;

template<typename Action, typename Env>
  struct apply_actor
{
  using type = typename Action::template result<Env>::type;
};

template<typename Eval>
  struct actor
    : Eval
{
  using eval_type = Eval;

  constexpr actor() = default;

  THRUST_HOST_DEVICE
  actor(const Eval &base);

  template <typename... Ts>
  THRUST_HOST_DEVICE
  typename apply_actor<eval_type, thrust::tuple<eval_ref<Ts>...>>::type
  operator()(Ts&&... ts) const;

  template<typename T>
  THRUST_HOST_DEVICE
  typename assign_result<Eval,T>::type
  operator=(const T &_1) const;
}; // end actor

// in general, as_actor should turn things into values
template<typename T>
  struct as_actor
{
  using type = value<T>;

  static inline THRUST_HOST_DEVICE type convert(const T &x)
  {
    return val(x);
  } // end convert()
}; // end as_actor

// specialization for things which are already actors
template<typename Eval>
  struct as_actor<actor<Eval> >
{
  using type = actor<Eval>;

  static inline THRUST_HOST_DEVICE const type &convert(const actor<Eval> &x)
  {
    return x;
  } // end convert()
}; // end as_actor

template<typename T>
  typename as_actor<T>::type
  THRUST_HOST_DEVICE
    make_actor(const T &x)
{
  return as_actor<T>::convert(x);
} // end make_actor()

} // end functional

// provide specializations for result_of for nullary, unary, and binary invocations of actor
template<typename Eval>
  struct result_of_adaptable_function<
    thrust::detail::functional::actor<Eval>()
  >
{
  using type =
    typename thrust::detail::functional::apply_actor<thrust::detail::functional::actor<Eval>, thrust::tuple<>>::type;
}; // end result_of

template<typename Eval, typename Arg1>
  struct result_of_adaptable_function<
    thrust::detail::functional::actor<Eval>(Arg1)
  >
{
  using type =
    typename thrust::detail::functional::apply_actor<thrust::detail::functional::actor<Eval>, thrust::tuple<Arg1>>::type;
}; // end result_of

template<typename Eval, typename Arg1, typename Arg2>
  struct result_of_adaptable_function<
    thrust::detail::functional::actor<Eval>(Arg1,Arg2)
  >
{
  using type = typename thrust::detail::functional::apply_actor<thrust::detail::functional::actor<Eval>,
                                                                thrust::tuple<Arg1, Arg2>>::type;
}; // end result_of

} // end detail
THRUST_NAMESPACE_END

#include <thrust/detail/functional/actor.inl>


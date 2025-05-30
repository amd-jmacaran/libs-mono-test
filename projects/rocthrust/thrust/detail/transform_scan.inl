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

#include <thrust/scan.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/system/detail/generic/select_system.h>
#include <thrust/system/detail/generic/transform_scan.h>
#include <thrust/system/detail/adl/transform_scan.h>

THRUST_NAMESPACE_BEGIN


THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename UnaryFunction,
         typename AssociativeOperator>
THRUST_HOST_DEVICE
  OutputIterator transform_inclusive_scan(const thrust::detail::execution_policy_base<DerivedPolicy> &exec,
                                          InputIterator first,
                                          InputIterator last,
                                          OutputIterator result,
                                          UnaryFunction unary_op,
                                          AssociativeOperator binary_op)
{
  using thrust::system::detail::generic::transform_inclusive_scan;
  return transform_inclusive_scan(thrust::detail::derived_cast(thrust::detail::strip_const(exec)), first, last, result, unary_op, binary_op);
} // end transform_inclusive_scan()


THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename UnaryFunction,
         typename T,
         typename AssociativeOperator>
THRUST_HOST_DEVICE
  OutputIterator transform_exclusive_scan(const thrust::detail::execution_policy_base<DerivedPolicy> &exec,
                                          InputIterator first,
                                          InputIterator last,
                                          OutputIterator result,
                                          UnaryFunction unary_op,
                                          T init,
                                          AssociativeOperator binary_op)
{
  using thrust::system::detail::generic::transform_exclusive_scan;
  return transform_exclusive_scan(thrust::detail::derived_cast(thrust::detail::strip_const(exec)), first, last, result, unary_op, init, binary_op);
} // end transform_exclusive_scan()


template<typename InputIterator,
         typename OutputIterator,
         typename UnaryFunction,
         typename BinaryFunction>
  OutputIterator transform_inclusive_scan(InputIterator first,
                                          InputIterator last,
                                          OutputIterator result,
                                          UnaryFunction unary_op,
                                          BinaryFunction binary_op)
{
  using thrust::system::detail::generic::select_system;

  using System1 = typename thrust::iterator_system<InputIterator>::type;
  using System2 = typename thrust::iterator_system<OutputIterator>::type;

  System1 system1;
  System2 system2;

  return thrust::transform_inclusive_scan(select_system(system1,system2), first, last, result, unary_op, binary_op);
} // end transform_inclusive_scan()


template<typename InputIterator,
         typename OutputIterator,
         typename UnaryFunction,
         typename T,
         typename AssociativeOperator>
  OutputIterator transform_exclusive_scan(InputIterator first,
                                          InputIterator last,
                                          OutputIterator result,
                                          UnaryFunction unary_op,
                                          T init,
                                          AssociativeOperator binary_op)
{
  using thrust::system::detail::generic::select_system;

  using System1 = typename thrust::iterator_system<InputIterator>::type;
  using System2 = typename thrust::iterator_system<OutputIterator>::type;

  System1 system1;
  System2 system2;

  return thrust::transform_exclusive_scan(select_system(system1,system2), first, last, result, unary_op, init, binary_op);
} // end transform_exclusive_scan()


THRUST_NAMESPACE_END


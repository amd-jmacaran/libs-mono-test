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

#include <thrust/gather.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/system/detail/generic/select_system.h>
#include <thrust/system/detail/generic/gather.h>
#include <thrust/system/detail/adl/gather.h>

THRUST_NAMESPACE_BEGIN

THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy,
         typename InputIterator,
         typename RandomAccessIterator,
         typename OutputIterator>
THRUST_HOST_DEVICE
  OutputIterator gather(const thrust::detail::execution_policy_base<DerivedPolicy> &exec,
                        InputIterator                                               map_first,
                        InputIterator                                               map_last,
                        RandomAccessIterator                                        input_first,
                        OutputIterator                                              result)
{
  using thrust::system::detail::generic::gather;
  return gather(thrust::detail::derived_cast(thrust::detail::strip_const(exec)), map_first, map_last, input_first, result);
} // end gather()


THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy,
         typename InputIterator1,
         typename InputIterator2,
         typename RandomAccessIterator,
         typename OutputIterator>
THRUST_HOST_DEVICE
  OutputIterator gather_if(const thrust::detail::execution_policy_base<DerivedPolicy> &exec,
                           InputIterator1                                              map_first,
                           InputIterator1                                              map_last,
                           InputIterator2                                              stencil,
                           RandomAccessIterator                                        input_first,
                           OutputIterator                                              result)
{
  using thrust::system::detail::generic::gather_if;
  return gather_if(thrust::detail::derived_cast(thrust::detail::strip_const(exec)), map_first, map_last, stencil, input_first, result);
} // end gather_if()


THRUST_EXEC_CHECK_DISABLE
template<typename DerivedPolicy,
         typename InputIterator1,
         typename InputIterator2,
         typename RandomAccessIterator,
         typename OutputIterator,
         typename Predicate>
THRUST_HOST_DEVICE
  OutputIterator gather_if(const thrust::detail::execution_policy_base<DerivedPolicy> &exec,
                           InputIterator1                                              map_first,
                           InputIterator1                                              map_last,
                           InputIterator2                                              stencil,
                           RandomAccessIterator                                        input_first,
                           OutputIterator                                              result,
                           Predicate                                                   pred)
{
  using thrust::system::detail::generic::gather_if;
  return gather_if(thrust::detail::derived_cast(thrust::detail::strip_const(exec)), map_first, map_last, stencil, input_first, result, pred);
} // end gather_if()


template<typename InputIterator,
         typename RandomAccessIterator,
         typename OutputIterator>
  OutputIterator gather(InputIterator        map_first,
                        InputIterator        map_last,
                        RandomAccessIterator input_first,
                        OutputIterator       result)
{
  using thrust::system::detail::generic::select_system;

  using System1 = typename thrust::iterator_system<InputIterator>::type;
  using System2 = typename thrust::iterator_system<RandomAccessIterator>::type;
  using System3 = typename thrust::iterator_system<OutputIterator>::type;

  System1 system1;
  System2 system2;
  System3 system3;

  return thrust::gather(select_system(system1,system2,system3), map_first, map_last, input_first, result);
} // end gather()


template<typename InputIterator1,
         typename InputIterator2,
         typename RandomAccessIterator,
         typename OutputIterator>
  OutputIterator gather_if(InputIterator1       map_first,
                           InputIterator1       map_last,
                           InputIterator2       stencil,
                           RandomAccessIterator input_first,
                           OutputIterator       result)
{
  using thrust::system::detail::generic::select_system;

  using System1 = typename thrust::iterator_system<InputIterator1>::type;
  using System2 = typename thrust::iterator_system<InputIterator2>::type;
  using System3 = typename thrust::iterator_system<RandomAccessIterator>::type;
  using System4 = typename thrust::iterator_system<OutputIterator>::type;

  System1 system1;
  System2 system2;
  System3 system3;
  System4 system4;

  return thrust::gather_if(select_system(system1,system2,system3,system4), map_first, map_last, stencil, input_first, result);
} // end gather_if()


template<typename InputIterator1,
         typename InputIterator2,
         typename RandomAccessIterator,
         typename OutputIterator,
         typename Predicate>
  OutputIterator gather_if(InputIterator1       map_first,
                           InputIterator1       map_last,
                           InputIterator2       stencil,
                           RandomAccessIterator input_first,
                           OutputIterator       result,
                           Predicate            pred)
{
  using thrust::system::detail::generic::select_system;

  using System1 = typename thrust::iterator_system<InputIterator1>::type;
  using System2 = typename thrust::iterator_system<InputIterator2>::type;
  using System3 = typename thrust::iterator_system<RandomAccessIterator>::type;
  using System4 = typename thrust::iterator_system<OutputIterator>::type;

  System1 system1;
  System2 system2;
  System3 system3;
  System4 system4;

  return thrust::gather_if(select_system(system1,system2,system3,system4), map_first, map_last, stencil, input_first, result, pred);
} // end gather_if()

THRUST_NAMESPACE_END

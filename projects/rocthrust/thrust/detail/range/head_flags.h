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
#include <thrust/iterator/transform_iterator.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/tuple.h>
#include <thrust/functional.h>
#include <thrust/sequence_access.h>


THRUST_NAMESPACE_BEGIN
namespace detail
{


template<typename RandomAccessIterator,
         typename BinaryPredicate = thrust::equal_to<typename thrust::iterator_value<RandomAccessIterator>::type>,
         typename ValueType = bool,
         typename IndexType = typename thrust::iterator_difference<RandomAccessIterator>::type>
  class head_flags_with_init
{
  using init_type = typename thrust::iterator_value<RandomAccessIterator>::type;

  // XXX WAR cudafe issue
  //private:
  public:
    struct head_flag_functor
    {
      BinaryPredicate binary_pred; // this must be the first member for performance reasons
      init_type init;
      IndexType n;

      using result_type = ValueType;

      THRUST_HOST_DEVICE
      head_flag_functor(init_type init, IndexType n)
        : binary_pred(), init(init), n(n)
      {}

      THRUST_HOST_DEVICE
      head_flag_functor(init_type init, IndexType n, BinaryPredicate binary_pred)
        : binary_pred(binary_pred), init(init), n(n)
      {}

      template<typename Tuple>
      THRUST_HOST_DEVICE THRUST_FORCEINLINE
      result_type operator()(const Tuple &t)
      {
        const IndexType i = thrust::get<0>(t);

        if(i == 0)
        {
          return !binary_pred(init, thrust::get<1>(t));
        }

        return !binary_pred(thrust::get<1>(t), thrust::get<2>(t));
      }
    };

    using counting_iterator = thrust::counting_iterator<IndexType>;

  public:
    using iterator = thrust::transform_iterator<
                      head_flag_functor,
                      thrust::zip_iterator<thrust::tuple<counting_iterator,RandomAccessIterator,RandomAccessIterator> >
                      >;

    THRUST_EXEC_CHECK_DISABLE
    THRUST_HOST_DEVICE
    head_flags_with_init(RandomAccessIterator first, RandomAccessIterator last, init_type init)
      : m_begin(thrust::make_transform_iterator(thrust::make_zip_iterator(thrust::make_tuple(thrust::counting_iterator<IndexType>(0), first, first - 1)),
                                                head_flag_functor(init, last - first))),
        m_end(m_begin + (last - first))
    {}

    THRUST_EXEC_CHECK_DISABLE
    THRUST_HOST_DEVICE
    head_flags_with_init(RandomAccessIterator first, RandomAccessIterator last, init_type init, BinaryPredicate binary_pred)
      : m_begin(thrust::make_transform_iterator(thrust::make_zip_iterator(thrust::make_tuple(thrust::counting_iterator<IndexType>(0), first, first - 1)),
                                                head_flag_functor(init, last - first, binary_pred))),
        m_end(m_begin + (last - first))
    {}

    THRUST_HOST_DEVICE
    iterator begin() const
    {
      return m_begin;
    }

    THRUST_HOST_DEVICE
    iterator end() const
    {
      return m_end;
    }

    template<typename OtherIndex>
    THRUST_HOST_DEVICE
    typename iterator::reference operator[](OtherIndex i)
    {
      return *(begin() + i);
    }

    THRUST_SYNTHESIZE_SEQUENCE_ACCESS(head_flags_with_init, iterator);

  private:
    iterator m_begin, m_end;
};



template<typename RandomAccessIterator,
         typename BinaryPredicate = thrust::equal_to<typename thrust::iterator_value<RandomAccessIterator>::type>,
         typename ValueType = bool,
         typename IndexType = typename thrust::iterator_difference<RandomAccessIterator>::type>
  class head_flags
{
  // XXX WAR cudafe issue
  //private:
  public:
    struct head_flag_functor
    {
      BinaryPredicate binary_pred; // this must be the first member for performance reasons
      IndexType n;

      using result_type = ValueType;

      THRUST_HOST_DEVICE
      head_flag_functor(IndexType n)
        : binary_pred(), n(n)
      {}

      THRUST_HOST_DEVICE
      head_flag_functor(IndexType n, BinaryPredicate binary_pred)
        : binary_pred(binary_pred), n(n)
      {}

      template<typename Tuple>
      THRUST_HOST_DEVICE THRUST_FORCEINLINE
      result_type operator()(const Tuple &t)
      {
        const IndexType i = thrust::get<0>(t);

        // note that we do not dereference the tuple's 2nd element when i <= 0
        // and therefore do not dereference a bad location at the boundary
        return (i == 0 || !binary_pred(thrust::get<1>(t), thrust::get<2>(t)));
      }
    };

    using counting_iterator = thrust::counting_iterator<IndexType>;

  public:
    using iterator = thrust::transform_iterator<
                      head_flag_functor,
                      thrust::zip_iterator<thrust::tuple<counting_iterator,RandomAccessIterator,RandomAccessIterator> >
                      >;

    THRUST_HOST_DEVICE
    head_flags(RandomAccessIterator first, RandomAccessIterator last)
      : m_begin(thrust::make_transform_iterator(thrust::make_zip_iterator(thrust::make_tuple(thrust::counting_iterator<IndexType>(0), first, first - 1)),
                                                head_flag_functor(last - first))),
        m_end(m_begin + (last - first))
    {}

    THRUST_HOST_DEVICE
    head_flags(RandomAccessIterator first, RandomAccessIterator last, BinaryPredicate binary_pred)
      : m_begin(thrust::make_transform_iterator(thrust::make_zip_iterator(thrust::make_tuple(thrust::counting_iterator<IndexType>(0), first, first - 1)),
                                                head_flag_functor(last - first, binary_pred))),
        m_end(m_begin + (last - first))
    {}

    THRUST_HOST_DEVICE
    iterator begin() const
    {
      return m_begin;
    }

    THRUST_HOST_DEVICE
    iterator end() const
    {
      return m_end;
    }

    template<typename OtherIndex>
    THRUST_HOST_DEVICE
    typename iterator::reference operator[](OtherIndex i)
    {
      return *(begin() + i);
    }

  private:
    iterator m_begin, m_end;
};


template<typename RandomAccessIterator, typename BinaryPredicate>
THRUST_HOST_DEVICE
head_flags<RandomAccessIterator, BinaryPredicate>
  make_head_flags(RandomAccessIterator first, RandomAccessIterator last, BinaryPredicate binary_pred)
{
  return head_flags<RandomAccessIterator, BinaryPredicate>(first, last, binary_pred);
}


template<typename RandomAccessIterator>
THRUST_HOST_DEVICE
head_flags<RandomAccessIterator>
  make_head_flags(RandomAccessIterator first, RandomAccessIterator last)
{
  return head_flags<RandomAccessIterator>(first, last);
}


} // end detail
THRUST_NAMESPACE_END


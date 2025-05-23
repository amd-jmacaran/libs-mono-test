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

#include <unittest/unittest.h>
#include <thrust/detail/static_assert.h>
#include <iterator>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <string>
#if THRUST_CPP_DIALECT >= 2017
  #include <string_view>
#endif
#include <deque>
#include <list>
#include <map>
#include <set>
#include <thrust/device_ptr.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/type_traits/is_contiguous_iterator.h>

THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
  std::string::iterator
>::value));

THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
  std::wstring::iterator
>::value));

#if THRUST_CPP_DIALECT >= 2017
THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
  std::string_view::iterator
>::value));

THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
  std::wstring_view::iterator
>::value));
#endif

THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
  std::vector<bool>::iterator
>::value));

template <typename T>
THRUST_HOST
void test_is_contiguous_iterator()
{
  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    T*
  >::value));

  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    T const*
  >::value));

  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    thrust::device_ptr<T>
  >::value));

  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    typename std::vector<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::vector<T>::reverse_iterator
  >::value));

  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    typename std::array<T, 1>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::list<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::deque<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::set<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::multiset<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::map<T, T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::multimap<T, T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::unordered_set<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::unordered_multiset<T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::unordered_map<T, T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    typename std::unordered_multimap<T, T>::iterator
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    std::istream_iterator<T>
  >::value));

  THRUST_STATIC_ASSERT((!thrust::is_contiguous_iterator<
    std::ostream_iterator<T>
  >::value));
}
DECLARE_GENERIC_UNITTEST(test_is_contiguous_iterator);

template <typename Vector>
THRUST_HOST
void test_is_contiguous_iterator_vectors()
{
  THRUST_STATIC_ASSERT((thrust::is_contiguous_iterator<
    typename Vector::iterator
  >::value));
}
DECLARE_VECTOR_UNITTEST(test_is_contiguous_iterator_vectors);


struct expect_pointer{};
struct expect_passthrough{};

template <typename IteratorT,
          typename PointerT,
          typename expected_unwrapped_type /* = expect_[pointer|passthrough] */>
struct check_unwrapped_iterator
{
  using unwrapped_t = typename std::remove_reference<
    decltype(thrust::try_unwrap_contiguous_iterator(
      std::declval<IteratorT>()))>::type;

  static constexpr bool value =
    std::is_same<expected_unwrapped_type, expect_pointer>::value
      ? std::is_same<unwrapped_t, PointerT>::value
      : std::is_same<unwrapped_t, IteratorT>::value;
};

template <typename T>
void test_try_unwrap_contiguous_iterator()
{
  // Raw pointers should pass whether expecting pointers or passthrough.
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<T *,
                                                 T *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<T *,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<T const *,
                                                 T const *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<T const *,
                                                 T const *,
                                                 expect_passthrough>::value));

  THRUST_STATIC_ASSERT((check_unwrapped_iterator<thrust::device_ptr<T>,
                                                 T *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<thrust::device_ptr<T const>,
                                                 T const *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::vector<T>::iterator,
                                                 T *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::vector<T>::reverse_iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::array<T, 1>::iterator,
                                                 T *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::array<T const, 1>::iterator,
                                                 T const *,
                                                 expect_pointer>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::list<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::deque<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::set<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::multiset<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::map<T, T>::iterator,
                                                 std::pair<T const, T> *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::multimap<T, T>::iterator,
                                                 std::pair<T const, T> *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::unordered_set<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::unordered_multiset<T>::iterator,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::unordered_map<T, T>::iterator,
                                                 std::pair<T const, T> *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<typename std::unordered_multimap<T, T>::iterator,
                                                 std::pair<T const, T> *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<std::istream_iterator<T>,
                                                 T *,
                                                 expect_passthrough>::value));
  THRUST_STATIC_ASSERT((check_unwrapped_iterator<std::ostream_iterator<T>,
                                                 void,
                                                 expect_passthrough>::value));
}
DECLARE_GENERIC_UNITTEST(test_try_unwrap_contiguous_iterator);

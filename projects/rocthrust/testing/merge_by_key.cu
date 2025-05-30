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
#include <thrust/merge.h>
#include <thrust/functional.h>
#include <thrust/sort.h>
#include <thrust/unique.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/iterator/retag.h>

template<typename Vector>
void TestMergeByKeySimple(void)
{
  const Vector a_key{0, 2, 4}, a_val{13, 7, 42}, b_key{0, 3, 3, 4}, b_val{42, 42, 7, 13};
  Vector ref_key{0, 0, 2, 3, 3, 4, 4}, ref_val{13, 42, 7, 42, 7, 42, 13};

  Vector result_key(7), result_val(7);

  const auto ends = thrust::merge_by_key(
    a_key.begin(),
    a_key.end(),
    b_key.begin(),
    b_key.end(),
    a_val.begin(),
    b_val.begin(),
    result_key.begin(),
    result_val.begin());

  ASSERT_EQUAL_QUIET(result_key.end(), ends.first);
  ASSERT_EQUAL_QUIET(result_val.end(), ends.second);
  ASSERT_EQUAL(ref_key, result_key);
  ASSERT_EQUAL(ref_val, result_val);
}
DECLARE_VECTOR_UNITTEST(TestMergeByKeySimple);


template<typename InputIterator1,
         typename InputIterator2,
         typename InputIterator3,
         typename InputIterator4,
         typename OutputIterator1,
         typename OutputIterator2>
  thrust::pair<OutputIterator1,OutputIterator2>
    merge_by_key(my_system &system,
                 InputIterator1,
                 InputIterator1,
                 InputIterator2,
                 InputIterator2,
                 InputIterator3,
                 InputIterator4,
                 OutputIterator1 keys_result,
                 OutputIterator2 values_result)
{
  system.validate_dispatch();
  return thrust::make_pair(keys_result, values_result);
}

void TestMergeByKeyDispatchExplicit()
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::merge_by_key(sys,
                       vec.begin(),
                       vec.begin(),
                       vec.begin(),
                       vec.begin(),
                       vec.begin(),
                       vec.begin(),
                       vec.begin(),
                       vec.begin());

  ASSERT_EQUAL(true, sys.is_valid());
}
DECLARE_UNITTEST(TestMergeByKeyDispatchExplicit);


template<typename InputIterator1,
         typename InputIterator2,
         typename InputIterator3,
         typename InputIterator4,
         typename OutputIterator1,
         typename OutputIterator2>
  thrust::pair<OutputIterator1,OutputIterator2>
    merge_by_key(my_tag,
                 InputIterator1,
                 InputIterator1,
                 InputIterator2,
                 InputIterator2,
                 InputIterator3,
                 InputIterator4,
                 OutputIterator1 keys_result,
                 OutputIterator2 values_result)
{
  *keys_result = 13;
  return thrust::make_pair(keys_result, values_result);
}

void TestMergeByKeyDispatchImplicit()
{
  thrust::device_vector<int> vec(1);

  thrust::merge_by_key(thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()),
                       thrust::retag<my_tag>(vec.begin()));

  ASSERT_EQUAL(13, vec.front());
}

template <typename T, typename CompareOp, typename... Args>
auto call_merge_by_key(Args&&... args) -> decltype(thrust::merge_by_key(std::forward<Args>(args)...))
{
  THRUST_IF_CONSTEXPR (std::is_void<CompareOp>::value)
  {
    return thrust::merge_by_key(std::forward<Args>(args)...);
  }
  else
  {
    // TODO(bgruber): remove next line in C++17 and pass CompareOp{} directly to stable_sort
    using C = std::conditional_t<std::is_void<CompareOp>::value, thrust::less<T>, CompareOp>;
    return thrust::merge_by_key(std::forward<Args>(args)..., C{});

  }
}

DECLARE_UNITTEST(TestMergeByKeyDispatchImplicit);

template <typename T, typename CompareOp = void>
void TestMergeByKey(size_t n)
{
  const auto random_keys = unittest::random_integers<unittest::int8_t>(n);
  const auto random_vals = unittest::random_integers<unittest::int8_t>(n);

  const size_t denominators[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (const auto& denom : denominators)
  {
    const size_t size_a = n / denom;

    thrust::host_vector<T> h_a_keys(random_keys.begin(), random_keys.begin() + size_a);
    thrust::host_vector<T> h_b_keys(random_keys.begin() + size_a, random_keys.end());

    const thrust::host_vector<T> h_a_vals(random_vals.begin(), random_vals.begin() + size_a);
    const thrust::host_vector<T> h_b_vals(random_vals.begin() + size_a, random_vals.end());

    THRUST_IF_CONSTEXPR (std::is_void<CompareOp>::value)
    {
      thrust::stable_sort(h_a_keys.begin(), h_a_keys.end());
      thrust::stable_sort(h_b_keys.begin(), h_b_keys.end());
    }
    else
    {
      // TODO(bgruber): remove next line in C++17 and pass CompareOp{} directly to stable_sort
      using C = std::conditional_t<std::is_void<CompareOp>::value, thrust::less<T>, CompareOp>;
      thrust::stable_sort(h_a_keys.begin(), h_a_keys.end(), C{});
      thrust::stable_sort(h_b_keys.begin(), h_b_keys.end(), C{});
    }

    const thrust::device_vector<T> d_a_keys = h_a_keys;
    const thrust::device_vector<T> d_b_keys = h_b_keys;

    const thrust::device_vector<T> d_a_vals = h_a_vals;
    const thrust::device_vector<T> d_b_vals = h_b_vals;

    thrust::host_vector<T> h_result_keys(n);
    thrust::host_vector<T> h_result_vals(n);

    thrust::device_vector<T> d_result_keys(n);
    thrust::device_vector<T> d_result_vals(n);

    const auto h_end = call_merge_by_key<T, CompareOp>(
      h_a_keys.begin(),
      h_a_keys.end(),
      h_b_keys.begin(),
      h_b_keys.end(),
      h_a_vals.begin(),
      h_b_vals.begin(),
      h_result_keys.begin(),
      h_result_vals.begin());

    h_result_keys.erase(h_end.first, h_result_keys.end());
    h_result_vals.erase(h_end.second, h_result_vals.end());

    const auto d_end = call_merge_by_key<T, CompareOp>(
      d_a_keys.begin(),
      d_a_keys.end(),
      d_b_keys.begin(),
      d_b_keys.end(),
      d_a_vals.begin(),
      d_b_vals.begin(),
      d_result_keys.begin(),
      d_result_vals.begin());
    d_result_keys.erase(d_end.first, d_result_keys.end());
    d_result_vals.erase(d_end.second, d_result_vals.end());

    ASSERT_EQUAL(h_result_keys, d_result_keys);
    ASSERT_EQUAL(h_result_vals, d_result_vals);
    ASSERT_EQUAL(true, h_end.first == h_result_keys.end());
    ASSERT_EQUAL(true, h_end.second == h_result_vals.end());
    ASSERT_EQUAL(true, d_end.first == d_result_keys.end());
    ASSERT_EQUAL(true, d_end.second == d_result_vals.end());
  }
}
DECLARE_VARIABLE_UNITTEST(TestMergeByKey);


template<typename T>
  void TestMergeByKeyToDiscardIterator(size_t n)
{
  auto h_a_keys = unittest::random_integers<T>(n);
  auto h_b_keys = unittest::random_integers<T>(n);

  const auto h_a_vals = unittest::random_integers<T>(n);
  const auto h_b_vals = unittest::random_integers<T>(n);

  thrust::stable_sort(h_a_keys.begin(), h_a_keys.end());
  thrust::stable_sort(h_b_keys.begin(), h_b_keys.end());

  const thrust::device_vector<T> d_a_keys = h_a_keys;
  const thrust::device_vector<T> d_b_keys = h_b_keys;

  const thrust::device_vector<T> d_a_vals = h_a_vals;
  const thrust::device_vector<T> d_b_vals = h_b_vals;

  using discard_pair = thrust::pair<thrust::discard_iterator<>, thrust::discard_iterator<>>;

  const discard_pair h_result = thrust::merge_by_key(
    h_a_keys.begin(),
    h_a_keys.end(),
    h_b_keys.begin(),
    h_b_keys.end(),
    h_a_vals.begin(),
    h_b_vals.begin(),
    thrust::make_discard_iterator(),
    thrust::make_discard_iterator());

  const discard_pair d_result = thrust::merge_by_key(
    d_a_keys.begin(),
    d_a_keys.end(),
    d_b_keys.begin(),
    d_b_keys.end(),
    d_a_vals.begin(),
    d_b_vals.begin(),
    thrust::make_discard_iterator(),
    thrust::make_discard_iterator());

  const thrust::discard_iterator<> reference(2 * n);

  ASSERT_EQUAL_QUIET(reference, h_result.first);
  ASSERT_EQUAL_QUIET(reference, h_result.second);
  ASSERT_EQUAL_QUIET(reference, d_result.first);
  ASSERT_EQUAL_QUIET(reference, d_result.second);
}
DECLARE_VARIABLE_UNITTEST(TestMergeByKeyToDiscardIterator);


template<typename T>
  void TestMergeByKeyDescending(size_t n)
{
  TestMergeByKey<T, thrust::greater<T>>(n);
}
DECLARE_VARIABLE_UNITTEST(TestMergeByKeyDescending);


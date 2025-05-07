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
#include <thrust/functional.h>
#include <thrust/transform.h>
#include <thrust/iterator/constant_iterator.h>

#include <thrust/detail/allocator/allocator_traits.h>

static const size_t num_samples = 10000;

template<typename Vector, typename U> struct rebind_vector;

template<typename T, typename U, typename Allocator>
  struct rebind_vector<thrust::host_vector<T, Allocator>, U>
{
  using alloc_traits = typename thrust::detail::allocator_traits<Allocator>;
  using new_alloc    = typename alloc_traits::template rebind_alloc<U>;
  using type         = thrust::host_vector<U, new_alloc>;
};

template<typename T, typename U, typename Allocator>
  struct rebind_vector<thrust::device_vector<T, Allocator>, U>
{
  using type = thrust::device_vector<U, typename Allocator::template rebind<U>::other>;
};

template<typename T, typename U, typename Allocator>
  struct rebind_vector<thrust::universal_vector<T, Allocator>, U>
{
  using type = thrust::universal_vector<U,
    typename Allocator::template rebind<U>::other>;
};

#define BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(name, op, reference_functor, type_list) \
template<typename Vector> \
  struct TestFunctionalPlaceholders##name \
{ \
  void operator()(const size_t) \
  { \
    constexpr size_t NUM_SAMPLES = 10000; \
    constexpr size_t ZERO = 0; \
    using T = typename Vector::value_type; \
    Vector lhs = unittest::random_samples<T>(NUM_SAMPLES); \
    Vector rhs = unittest::random_samples<T>(NUM_SAMPLES); \
    thrust::replace(rhs.begin(), rhs.end(), T(0), T(1)); \
\
    Vector reference(lhs.size()); \
    Vector result(lhs.size()); \
    using namespace thrust::placeholders; \
\
    thrust::transform(lhs.begin(), lhs.end(), rhs.begin(), reference.begin(), reference_functor<T>()); \
    thrust::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), _1 op _2); \
    ASSERT_ALMOST_EQUAL(reference, result); \
\
    thrust::transform(lhs.begin(), lhs.end(), thrust::make_constant_iterator<T>(1), reference.begin(), reference_functor<T>()); \
    thrust::transform(lhs.begin(), lhs.end(), result.begin(), _1 op T(1)); \
    ASSERT_ALMOST_EQUAL(reference, result); \
\
    thrust::transform(thrust::make_constant_iterator<T>(1,ZERO), thrust::make_constant_iterator<T>(1,NUM_SAMPLES), rhs.begin(), reference.begin(), reference_functor<T>()); \
    thrust::transform(rhs.begin(), rhs.end(), result.begin(), T(1) op _1); \
    ASSERT_ALMOST_EQUAL(reference, result); \
  } \
}; \
VectorUnitTest<TestFunctionalPlaceholders##name, type_list, thrust::device_vector, thrust::device_allocator> TestFunctionalPlaceholders##name##DeviceInstance; \
VectorUnitTest<TestFunctionalPlaceholders##name, type_list, thrust::host_vector, std::allocator> TestFunctionalPlaceholders##name##HostInstance;

BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(BitAnd, &, thrust::bit_and, SmallIntegralTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(BitOr,  |, thrust::bit_or,  SmallIntegralTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(BitXor, ^, thrust::bit_xor, SmallIntegralTypes);

template<typename T>
  struct bit_negate_reference
{
  THRUST_HOST_DEVICE T operator()(const T &x) const
  {
    return ~x;
  }
};

template<typename Vector>
  void TestFunctionalPlaceholdersBitNegate(void)
{
  using T = typename Vector::value_type;
  using bool_vector = typename rebind_vector<Vector,bool>::type;
  Vector input = unittest::random_samples<T>(num_samples);

  bool_vector reference(input.size());
  thrust::transform(input.begin(), input.end(), reference.begin(), bit_negate_reference<T>());

  using namespace thrust::placeholders;
  bool_vector result(input.size());
  thrust::transform(input.begin(), input.end(), result.begin(), ~_1);

  ASSERT_EQUAL(reference, result);
}
DECLARE_INTEGRAL_VECTOR_UNITTEST(TestFunctionalPlaceholdersBitNegate);


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

#define BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(name, op, reference_functor, type_list) \
template<typename Vector> \
  struct TestFunctionalPlaceholders##name \
{ \
  void operator()(const size_t) \
  { \
    static const size_t num_samples = 10000; \
    const size_t zero = 0; \
    using T = typename Vector::value_type; \
    Vector lhs = unittest::random_samples<T>(num_samples); \
    Vector rhs = unittest::random_samples<T>(num_samples); \
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
    thrust::transform(thrust::make_constant_iterator<T>(1,zero), thrust::make_constant_iterator<T>(1,num_samples), rhs.begin(), reference.begin(), reference_functor<T>()); \
    thrust::transform(rhs.begin(), rhs.end(), result.begin(), T(1) op _1); \
    ASSERT_ALMOST_EQUAL(reference, result); \
  } \
}; \
VectorUnitTest<TestFunctionalPlaceholders##name, type_list, thrust::device_vector, thrust::device_allocator> TestFunctionalPlaceholders##name##DeviceInstance; \
VectorUnitTest<TestFunctionalPlaceholders##name, type_list, thrust::host_vector, std::allocator> TestFunctionalPlaceholders##name##HostInstance;

BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(Plus,       +, thrust::plus,       ThirtyTwoBitTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(Minus,      -, thrust::minus,      ThirtyTwoBitTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(Multiplies, *, thrust::multiplies, ThirtyTwoBitTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(Divides,    /, thrust::divides,    ThirtyTwoBitTypes);
BINARY_FUNCTIONAL_PLACEHOLDERS_TEST(Modulus,    %, thrust::modulus,    SmallIntegralTypes);

#define UNARY_FUNCTIONAL_PLACEHOLDERS_TEST(name, reference_operator, functor) \
template<typename Vector> \
  void TestFunctionalPlaceholders##name(void) \
{ \
  static const size_t num_samples = 10000; \
  using T = typename Vector::value_type; \
  Vector input = unittest::random_samples<T>(num_samples); \
\
  Vector reference(input.size()); \
  thrust::transform(input.begin(), input.end(), reference.begin(), functor<T>()); \
\
  using namespace thrust::placeholders; \
  Vector result(input.size()); \
  thrust::transform(input.begin(), input.end(), result.begin(), reference_operator _1); \
\
  ASSERT_EQUAL(reference, result); \
} \
DECLARE_VECTOR_UNITTEST(TestFunctionalPlaceholders##name);

template<typename T>
  struct unary_plus_reference
{
  THRUST_HOST_DEVICE T operator()(const T &x) const
  { // Static cast to undo integral promotion
    return static_cast<T>(+x);
  }
};

UNARY_FUNCTIONAL_PLACEHOLDERS_TEST(UnaryPlus, +, unary_plus_reference);
UNARY_FUNCTIONAL_PLACEHOLDERS_TEST(Negate,    -, thrust::negate);


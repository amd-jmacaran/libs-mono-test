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
#include <thrust/detail/config.h>
#include <thrust/device_malloc_allocator.h>
#include <thrust/system/cpp/vector.h>

#include <thrust/detail/nv_target.h>

#include <memory>

template <typename T>
struct my_allocator_with_custom_construct1
  : thrust::device_malloc_allocator<T>
{
  THRUST_HOST_DEVICE
  my_allocator_with_custom_construct1()
  {}

  THRUST_HOST_DEVICE
  void construct(T *p)
  {
    *p = 13;
  }
};

template <typename T>
void TestAllocatorCustomDefaultConstruct(size_t n)
{
  thrust::device_vector<T> ref(n, 13);
  thrust::device_vector<T, my_allocator_with_custom_construct1<T> > vec(n);

  ASSERT_EQUAL_QUIET(ref, vec);
}
DECLARE_VARIABLE_UNITTEST(TestAllocatorCustomDefaultConstruct);

template <typename T>
struct my_allocator_with_custom_construct2
  : thrust::device_malloc_allocator<T>
{
  THRUST_HOST_DEVICE
  my_allocator_with_custom_construct2()
  {}

  template <typename Arg>
  THRUST_HOST_DEVICE
  void construct(T *p, const Arg &)
  {
    *p = 13;
  }
};

template <typename T>
void TestAllocatorCustomCopyConstruct(size_t n)
{
  thrust::device_vector<T> ref(n, 13);
  thrust::device_vector<T> copy_from(n, 7);
  thrust::device_vector<T, my_allocator_with_custom_construct2<T> >
    vec(copy_from.begin(), copy_from.end());

  ASSERT_EQUAL_QUIET(ref, vec);
}
DECLARE_VARIABLE_UNITTEST(TestAllocatorCustomCopyConstruct);

template <typename T>
struct my_allocator_with_custom_destroy
{
  // This is only used with thrust::cpp::vector:
  using system_type = thrust::cpp::tag;

  using value_type = T;
  using reference = T &;
  using const_reference = const T &;

  static bool g_state;

  THRUST_HOST
  my_allocator_with_custom_destroy(){}

  THRUST_HOST
  my_allocator_with_custom_destroy(const my_allocator_with_custom_destroy &other)
    : use_me_to_alloc(other.use_me_to_alloc)
  {}

  THRUST_HOST
  ~my_allocator_with_custom_destroy(){}

  THRUST_HOST_DEVICE
  void destroy(T *)
  {
    NV_IF_TARGET(NV_IS_HOST, (g_state = true;));
  }

  value_type *allocate(std::ptrdiff_t n)
  {
    return use_me_to_alloc.allocate(n);
  }

  void deallocate(value_type *ptr, std::ptrdiff_t n)
  {
    use_me_to_alloc.deallocate(ptr,n);
  }

  bool operator==(const my_allocator_with_custom_destroy &) const
  {
    return true;
  }

  bool operator!=(const my_allocator_with_custom_destroy &other) const
  {
    return !(*this == other);
  }

  using is_always_equal = thrust::detail::true_type;

  // use composition rather than inheritance
  // to avoid inheriting std::allocator's member
  // function destroy
  std::allocator<T> use_me_to_alloc;
};

template <typename T>
bool my_allocator_with_custom_destroy<T>::g_state = false;

template <typename T>
void TestAllocatorCustomDestroy(size_t n)
{
  my_allocator_with_custom_destroy<T>::g_state = false;

  {
    thrust::cpp::vector<T, my_allocator_with_custom_destroy<T> > vec(n);
  } // destroy everything

  // state should only be true when there are values to destroy:
  ASSERT_EQUAL(n > 0, my_allocator_with_custom_destroy<T>::g_state);
}
DECLARE_VARIABLE_UNITTEST(TestAllocatorCustomDestroy);

template <typename T>
struct my_minimal_allocator
{
  using value_type = T;

  // XXX ideally, we shouldn't require these two aliases
  using reference       = T&;
  using const_reference = const T&;

  THRUST_HOST
  my_minimal_allocator(){}

  THRUST_HOST
  my_minimal_allocator(const my_minimal_allocator &other)
    : use_me_to_alloc(other.use_me_to_alloc)
  {}

  THRUST_HOST
  ~my_minimal_allocator(){}

  value_type *allocate(std::ptrdiff_t n)
  {
    return use_me_to_alloc.allocate(n);
  }

  void deallocate(value_type *ptr, std::ptrdiff_t n)
  {
    use_me_to_alloc.deallocate(ptr,n);
  }

  std::allocator<T> use_me_to_alloc;
};

template <typename T>
void TestAllocatorMinimal(size_t n)
{
  thrust::cpp::vector<int, my_minimal_allocator<int> > vec(n, 13);

  // XXX copy to h_vec because ASSERT_EQUAL doesn't know about cpp::vector
  thrust::host_vector<int> h_vec(vec.begin(), vec.end());
  thrust::host_vector<int> ref(n, 13);

  ASSERT_EQUAL(ref, h_vec);
}
DECLARE_VARIABLE_UNITTEST(TestAllocatorMinimal);

void TestAllocatorTraitsRebind()
{
  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        thrust::device_malloc_allocator<int>
      >::template rebind_traits<float>::other,
      typename thrust::detail::allocator_traits<
        thrust::device_malloc_allocator<float>
      >
    >::value),
    true
  );

  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        my_minimal_allocator<int>
      >::template rebind_traits<float>::other,
      typename thrust::detail::allocator_traits<
        my_minimal_allocator<float>
      >
    >::value),
    true
  );
}
DECLARE_UNITTEST(TestAllocatorTraitsRebind);

void TestAllocatorTraitsRebindCpp11()
{
  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        thrust::device_malloc_allocator<int>
      >::template rebind_alloc<float>,
      thrust::device_malloc_allocator<float>
    >::value),
    true
  );

  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        my_minimal_allocator<int>
      >::template rebind_alloc<float>,
      my_minimal_allocator<float>
    >::value),
    true
  );

  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        thrust::device_malloc_allocator<int>
      >::template rebind_traits<float>,
      typename thrust::detail::allocator_traits<
        thrust::device_malloc_allocator<float>
      >
    >::value),
    true
  );

  ASSERT_EQUAL(
    (thrust::detail::is_same<
      typename thrust::detail::allocator_traits<
        my_minimal_allocator<int>
      >::template rebind_traits<float>,
      typename thrust::detail::allocator_traits<
        my_minimal_allocator<float>
      >
    >::value),
    true
  );
}
DECLARE_UNITTEST(TestAllocatorTraitsRebindCpp11);

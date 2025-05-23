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
#include <thrust/sort.h>
#include <thrust/execution_policy.h>


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy, typename Iterator1, typename Iterator2>
__global__
void is_sorted_until_kernel(ExecutionPolicy exec, Iterator1 first, Iterator1 last, Iterator2 result)
{
  *result = thrust::is_sorted_until(exec, first, last);
}


template<typename ExecutionPolicy>
void TestIsSortedUntilDevice(ExecutionPolicy exec)
{
  size_t n = 1000;

  thrust::device_vector<int> v = unittest::random_integers<int>(n);

  using iter_type = typename thrust::device_vector<int>::iterator;

  thrust::device_vector<iter_type> result(1);

  v[0] = 1;
  v[1] = 0;
  
  is_sorted_until_kernel<<<1,1>>>(exec, v.begin(), v.end(), result.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  ASSERT_EQUAL_QUIET(v.begin() + 1, (iter_type)result[0]);
  
  thrust::sort(v.begin(), v.end());
  
  is_sorted_until_kernel<<<1,1>>>(exec, v.begin(), v.end(), result.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  ASSERT_EQUAL_QUIET(v.end(), (iter_type)result[0]);
}


void TestIsSortedUntilDeviceSeq()
{
  TestIsSortedUntilDevice(thrust::seq);
}
DECLARE_UNITTEST(TestIsSortedUntilDeviceSeq);


void TestIsSortedUntilDeviceDevice()
{
  TestIsSortedUntilDevice(thrust::device);
}
DECLARE_UNITTEST(TestIsSortedUntilDeviceDevice);
#endif


void TestIsSortedUntilCudaStreams()
{
  using Vector = thrust::device_vector<int>;

  using T        = Vector::value_type;
  using Iterator = Vector::iterator;

  cudaStream_t s;
  cudaStreamCreate(&s);

  Vector v(4);
  v[0] = 0; v[1] = 5; v[2] = 8; v[3] = 0;

  Iterator first = v.begin();

  Iterator last  = v.begin() + 0;
  Iterator ref = last;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last));

  last = v.begin() + 1;
  ref = last;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last));

  last = v.begin() + 2;
  ref = last;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last));

  last = v.begin() + 3;
  ref = v.begin() + 3;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last));

  last = v.begin() + 4;
  ref = v.begin() + 3;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last));

  last = v.begin() + 3;
  ref = v.begin() + 3;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last, thrust::less<T>()));

  last = v.begin() + 4;
  ref = v.begin() + 3;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last, thrust::less<T>()));

  last = v.begin() + 1;
  ref = v.begin() + 1;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last, thrust::greater<T>()));

  last = v.begin() + 4;
  ref = v.begin() + 1;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last, thrust::greater<T>()));

  first = v.begin() + 2;
  last = v.begin() + 4;
  ref = v.begin() + 4;
  ASSERT_EQUAL_QUIET(ref, thrust::is_sorted_until(thrust::cuda::par.on(s), first, last, thrust::greater<T>()));

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestIsSortedUntilCudaStreams);


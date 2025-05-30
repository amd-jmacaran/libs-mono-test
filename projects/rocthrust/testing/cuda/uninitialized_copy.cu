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
#include <thrust/uninitialized_copy.h>
#include <thrust/execution_policy.h>


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy, typename Iterator1, typename Iterator2>
__global__
void uninitialized_copy_kernel(ExecutionPolicy exec, Iterator1 first, Iterator1 last, Iterator2 result)
{
  thrust::uninitialized_copy(exec, first, last, result);
}


template<typename ExecutionPolicy>
void TestUninitializedCopyDevice(ExecutionPolicy exec)
{
  using Vector = thrust::device_vector<int>;

  Vector v1(5);
  v1[0] = 0; v1[1] = 1; v1[2] = 2; v1[3] = 3; v1[4] = 4;
  
  // copy to Vector
  Vector v2(5);
  uninitialized_copy_kernel<<<1,1>>>(exec, v1.begin(), v1.end(), v2.begin());
  cudaError_t const err = cudaDeviceSynchronize();
  ASSERT_EQUAL(cudaSuccess, err);

  ASSERT_EQUAL(v2[0], 0);
  ASSERT_EQUAL(v2[1], 1);
  ASSERT_EQUAL(v2[2], 2);
  ASSERT_EQUAL(v2[3], 3);
  ASSERT_EQUAL(v2[4], 4);
}


void TestUninitializedCopyDeviceSeq()
{
  TestUninitializedCopyDevice(thrust::seq);
}
DECLARE_UNITTEST(TestUninitializedCopyDeviceSeq);


void TestUninitializedCopyDeviceDevice()
{
  TestUninitializedCopyDevice(thrust::device);
}
DECLARE_UNITTEST(TestUninitializedCopyDeviceDevice);
#endif


void TestUninitializedCopyCudaStreams()
{
  using Vector = thrust::device_vector<int>;

  Vector v1(5);
  v1[0] = 0; v1[1] = 1; v1[2] = 2; v1[3] = 3; v1[4] = 4;
  
  // copy to Vector
  Vector v2(5);

  cudaStream_t s;
  cudaStreamCreate(&s);

  thrust::uninitialized_copy(thrust::cuda::par.on(s), v1.begin(), v1.end(), v2.begin());
  cudaStreamSynchronize(s);

  ASSERT_EQUAL(v2[0], 0);
  ASSERT_EQUAL(v2[1], 1);
  ASSERT_EQUAL(v2[2], 2);
  ASSERT_EQUAL(v2[3], 3);
  ASSERT_EQUAL(v2[4], 4);

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestUninitializedCopyCudaStreams);


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy, typename Iterator1, typename Size, typename Iterator2>
__global__
void uninitialized_copy_n_kernel(ExecutionPolicy exec, Iterator1 first, Size n, Iterator2 result)
{
  thrust::uninitialized_copy_n(exec, first, n, result);
}


template<typename ExecutionPolicy>
void TestUninitializedCopyNDevice(ExecutionPolicy exec)
{
  using Vector = thrust::device_vector<int>;

  Vector v1(5);
  v1[0] = 0; v1[1] = 1; v1[2] = 2; v1[3] = 3; v1[4] = 4;
  
  // copy to Vector
  Vector v2(5);
  uninitialized_copy_n_kernel<<<1,1>>>(exec, v1.begin(), v1.size(), v2.begin());
  cudaError_t const err = cudaDeviceSynchronize();
  ASSERT_EQUAL(cudaSuccess, err);

  ASSERT_EQUAL(v2[0], 0);
  ASSERT_EQUAL(v2[1], 1);
  ASSERT_EQUAL(v2[2], 2);
  ASSERT_EQUAL(v2[3], 3);
  ASSERT_EQUAL(v2[4], 4);
}


void TestUninitializedCopyNDeviceSeq()
{
  TestUninitializedCopyNDevice(thrust::seq);
}
DECLARE_UNITTEST(TestUninitializedCopyNDeviceSeq);


void TestUninitializedCopyNDeviceDevice()
{
  TestUninitializedCopyNDevice(thrust::device);
}
DECLARE_UNITTEST(TestUninitializedCopyNDeviceDevice);
#endif


void TestUninitializedCopyNCudaStreams()
{
  using Vector = thrust::device_vector<int>;

  Vector v1(5);
  v1[0] = 0; v1[1] = 1; v1[2] = 2; v1[3] = 3; v1[4] = 4;
  
  // copy to Vector
  Vector v2(5);

  cudaStream_t s;
  cudaStreamCreate(&s);

  thrust::uninitialized_copy_n(thrust::cuda::par.on(s), v1.begin(), v1.size(), v2.begin());
  cudaStreamSynchronize(s);

  ASSERT_EQUAL(v2[0], 0);
  ASSERT_EQUAL(v2[1], 1);
  ASSERT_EQUAL(v2[2], 2);
  ASSERT_EQUAL(v2[3], 3);
  ASSERT_EQUAL(v2[4], 4);

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestUninitializedCopyNCudaStreams);


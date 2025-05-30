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
#include <thrust/transform_scan.h>
#include <thrust/execution_policy.h>


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy, typename Iterator1, typename Iterator2, typename Function1, typename Function2, typename Iterator3>
__global__
void transform_inclusive_scan_kernel(ExecutionPolicy exec, Iterator1 first, Iterator1 last, Iterator2 result1, Function1 f1, Function2 f2, Iterator3 result2)
{
  *result2 = thrust::transform_inclusive_scan(exec, first, last, result1, f1, f2);
}


template<typename ExecutionPolicy, typename Iterator1, typename Iterator2, typename Function1, typename T, typename Function2, typename Iterator3>
__global__
void transform_exclusive_scan_kernel(ExecutionPolicy exec, Iterator1 first, Iterator1 last, Iterator2 result, Function1 f1, T init, Function2 f2, Iterator3 result2)
{
  *result2 = thrust::transform_exclusive_scan(exec, first, last, result, f1, init, f2);
}


template<typename ExecutionPolicy>
void TestTransformScanDevice(ExecutionPolicy exec)
{
  using Vector = thrust::device_vector<int>;
  using T      = typename Vector::value_type;

  typename Vector::iterator iter;
  
  Vector input(5);
  Vector ref(5);
  Vector output(5);
  
  input[0] = 1; input[1] = 3; input[2] = -2; input[3] = 4; input[4] = -5;
  
  Vector input_copy(input);

  thrust::device_vector<typename Vector::iterator> iter_vec(1);
  
  // inclusive scan
  transform_inclusive_scan_kernel<<<1,1>>>(exec, input.begin(), input.end(), output.begin(), thrust::negate<T>(), thrust::plus<T>(), iter_vec.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  iter = iter_vec[0];
  ref[0] = -1; ref[1] = -4; ref[2] = -2; ref[3] = -6; ref[4] = -1;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(ref, output);
  
  // exclusive scan with 0 init
  transform_exclusive_scan_kernel<<<1,1>>>(exec, input.begin(), input.end(), output.begin(), thrust::negate<T>(), 0, thrust::plus<T>(), iter_vec.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  ref[0] = 0; ref[1] = -1; ref[2] = -4; ref[3] = -2; ref[4] = -6;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(ref, output);
  
  // exclusive scan with nonzero init
  transform_exclusive_scan_kernel<<<1,1>>>(exec, input.begin(), input.end(), output.begin(), thrust::negate<T>(), 3, thrust::plus<T>(), iter_vec.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  iter = iter_vec[0];
  ref[0] = 3; ref[1] = 2; ref[2] = -1; ref[3] = 1; ref[4] = -3;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(ref, output);
  
  // inplace inclusive scan
  input = input_copy;
  transform_inclusive_scan_kernel<<<1,1>>>(exec, input.begin(), input.end(), input.begin(), thrust::negate<T>(), thrust::plus<T>(), iter_vec.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  iter = iter_vec[0];
  ref[0] = -1; ref[1] = -4; ref[2] = -2; ref[3] = -6; ref[4] = -1;
  ASSERT_EQUAL(std::size_t(iter - input.begin()), input.size());
  ASSERT_EQUAL(ref, input);
  
  // inplace exclusive scan with init
  input = input_copy;
  transform_exclusive_scan_kernel<<<1,1>>>(exec, input.begin(), input.end(), input.begin(), thrust::negate<T>(), 3, thrust::plus<T>(), iter_vec.begin());
  {
    cudaError_t const err = cudaDeviceSynchronize();
    ASSERT_EQUAL(cudaSuccess, err);
  }

  iter = iter_vec[0];
  ref[0] = 3; ref[1] = 2; ref[2] = -1; ref[3] = 1; ref[4] = -3;
  ASSERT_EQUAL(std::size_t(iter - input.begin()), input.size());
  ASSERT_EQUAL(ref, input);
}


void TestTransformScanDeviceSeq()
{
  TestTransformScanDevice(thrust::seq);
}
DECLARE_UNITTEST(TestTransformScanDeviceSeq);


void TestTransformScanDeviceDevice()
{
  TestTransformScanDevice(thrust::device);
}
DECLARE_UNITTEST(TestTransformScanDeviceDevice);
#endif


void TestTransformScanCudaStreams()
{
  using Vector = thrust::device_vector<int>;
  using T      = Vector::value_type;

  Vector::iterator iter;

  Vector input(5);
  Vector result(5);
  Vector output(5);

  input[0] = 1; input[1] = 3; input[2] = -2; input[3] = 4; input[4] = -5;

  Vector input_copy(input);

  cudaStream_t s;
  cudaStreamCreate(&s);

  // inclusive scan
  iter = thrust::transform_inclusive_scan(thrust::cuda::par.on(s), input.begin(), input.end(), output.begin(), thrust::negate<T>(), thrust::plus<T>());
  cudaStreamSynchronize(s);

  result[0] = -1; result[1] = -4; result[2] = -2; result[3] = -6; result[4] = -1;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(output, result);
  
  // exclusive scan with 0 init
  iter = thrust::transform_exclusive_scan(thrust::cuda::par.on(s), input.begin(), input.end(), output.begin(), thrust::negate<T>(), 0, thrust::plus<T>());
  cudaStreamSynchronize(s);

  result[0] = 0; result[1] = -1; result[2] = -4; result[3] = -2; result[4] = -6;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(output, result);
  
  // exclusive scan with nonzero init
  iter = thrust::transform_exclusive_scan(thrust::cuda::par.on(s), input.begin(), input.end(), output.begin(), thrust::negate<T>(), 3, thrust::plus<T>());
  cudaStreamSynchronize(s);

  result[0] = 3; result[1] = 2; result[2] = -1; result[3] = 1; result[4] = -3;
  ASSERT_EQUAL(std::size_t(iter - output.begin()), input.size());
  ASSERT_EQUAL(input,  input_copy);
  ASSERT_EQUAL(output, result);
  
  // inplace inclusive scan
  input = input_copy;
  iter = thrust::transform_inclusive_scan(thrust::cuda::par.on(s), input.begin(), input.end(), input.begin(), thrust::negate<T>(), thrust::plus<T>());
  cudaStreamSynchronize(s);

  result[0] = -1; result[1] = -4; result[2] = -2; result[3] = -6; result[4] = -1;
  ASSERT_EQUAL(std::size_t(iter - input.begin()), input.size());
  ASSERT_EQUAL(input, result);

  // inplace exclusive scan with init
  input = input_copy;
  iter = thrust::transform_exclusive_scan(thrust::cuda::par.on(s), input.begin(), input.end(), input.begin(), thrust::negate<T>(), 3, thrust::plus<T>());
  cudaStreamSynchronize(s);

  result[0] = 3; result[1] = 2; result[2] = -1; result[3] = 1; result[4] = -3;
  ASSERT_EQUAL(std::size_t(iter - input.begin()), input.size());
  ASSERT_EQUAL(input, result);

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestTransformScanCudaStreams);

void TestTransformScanConstAccumulator()
{
  using Vector = thrust::device_vector<int>;
  using T      = Vector::value_type;

  Vector::iterator iter;

  Vector input(5);
  Vector reference(5);
  Vector output(5);

  input[0] = 1;
  input[1] = 3;
  input[2] = -2;
  input[3] = 4;
  input[4] = -5;

  thrust::transform_inclusive_scan(input.begin(),
                                   input.end(),
                                   output.begin(),
                                   thrust::identity<T>(),
                                   thrust::plus<T>());
  thrust::inclusive_scan(input.begin(), input.end(), reference.begin(), thrust::plus<T>());

  ASSERT_EQUAL(output, reference);
}
DECLARE_UNITTEST(TestTransformScanConstAccumulator);

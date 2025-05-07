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
#include <thrust/execution_policy.h>


#ifdef THRUST_TEST_DEVICE_SIDE
template<typename ExecutionPolicy,
         typename Iterator1,
         typename Iterator2,
         typename Iterator3,
         typename Iterator4,
         typename Iterator5,
         typename Iterator6,
         typename Iterator7>
__global__
void merge_by_key_kernel(ExecutionPolicy exec,
                         Iterator1 keys_first1, Iterator1 keys_last1,
                         Iterator2 keys_first2, Iterator2 keys_last2,
                         Iterator3 values_first1,
                         Iterator4 values_first2,
                         Iterator5 keys_result,
                         Iterator6 values_result,
                         Iterator7 result)
{
  *result = thrust::merge_by_key(exec, keys_first1, keys_last1, keys_first2, keys_last2, values_first1, values_first2, keys_result, values_result);
}


template<typename ExecutionPolicy>
void TestMergeByKeyDevice(ExecutionPolicy exec)
{
  thrust::device_vector<int> a_key(3), a_val(3), b_key(4), b_val(4);

  a_key[0] = 0;  a_key[1] = 2; a_key[2] = 4;
  a_val[0] = 13; a_val[1] = 7; a_val[2] = 42;

  b_key[0] = 0 ; b_key[1] = 3;  b_key[2] = 3; b_key[3] = 4;
  b_val[0] = 42; b_val[1] = 42; b_val[2] = 7; b_val[3] = 13;

  thrust::device_vector<int> ref_key(7), ref_val(7);
  ref_key[0] = 0; ref_val[0] = 13;
  ref_key[1] = 0; ref_val[1] = 42;
  ref_key[2] = 2; ref_val[2] = 7;
  ref_key[3] = 3; ref_val[3] = 42;
  ref_key[4] = 3; ref_val[4] = 7;
  ref_key[5] = 4; ref_val[5] = 42;
  ref_key[6] = 4; ref_val[6] = 13;

  thrust::device_vector<int> result_key(7), result_val(7);

  using Iterator = typename thrust::device_vector<int>::iterator;

  thrust::device_vector<thrust::pair<Iterator,Iterator> > result_ends(1);

  merge_by_key_kernel<<<1,1>>>(exec,
                               a_key.begin(), a_key.end(),
                               b_key.begin(), b_key.end(),
                               a_val.begin(), b_val.begin(),
                               result_key.begin(),
                               result_val.begin(),
                               result_ends.begin());
  cudaError_t const err = cudaDeviceSynchronize();
  ASSERT_EQUAL(cudaSuccess, err);

  thrust::pair<Iterator,Iterator> ends = result_ends[0];

  ASSERT_EQUAL_QUIET(result_key.end(), ends.first);
  ASSERT_EQUAL_QUIET(result_val.end(), ends.second);
  ASSERT_EQUAL(ref_key, result_key);
  ASSERT_EQUAL(ref_val, result_val);
}


void TestMergeByKeyDeviceSeq()
{
  TestMergeByKeyDevice(thrust::seq);
}
DECLARE_UNITTEST(TestMergeByKeyDeviceSeq);


void TestMergeByKeyDeviceDevice()
{
  TestMergeByKeyDevice(thrust::device);
}
DECLARE_UNITTEST(TestMergeByKeyDeviceDevice);
#endif


void TestMergeByKeyCudaStreams()
{
  using Vector   = thrust::device_vector<int>;
  using Iterator = Vector::iterator;

  Vector a_key(3), a_val(3), b_key(4), b_val(4);

  a_key[0] = 0;  a_key[1] = 2; a_key[2] = 4;
  a_val[0] = 13; a_val[1] = 7; a_val[2] = 42;

  b_key[0] = 0 ; b_key[1] = 3;  b_key[2] = 3; b_key[3] = 4;
  b_val[0] = 42; b_val[1] = 42; b_val[2] = 7; b_val[3] = 13;

  Vector ref_key(7), ref_val(7);
  ref_key[0] = 0; ref_val[0] = 13;
  ref_key[1] = 0; ref_val[1] = 42;
  ref_key[2] = 2; ref_val[2] = 7;
  ref_key[3] = 3; ref_val[3] = 42;
  ref_key[4] = 3; ref_val[4] = 7;
  ref_key[5] = 4; ref_val[5] = 42;
  ref_key[6] = 4; ref_val[6] = 13;

  Vector result_key(7), result_val(7);

  cudaStream_t s;
  cudaStreamCreate(&s);

  thrust::pair<Iterator,Iterator> ends =
    thrust::merge_by_key(thrust::cuda::par.on(s),
                         a_key.begin(), a_key.end(),
                         b_key.begin(), b_key.end(),
                         a_val.begin(), b_val.begin(),
                         result_key.begin(),
                         result_val.begin());

  cudaStreamSynchronize(s);

  ASSERT_EQUAL_QUIET(result_key.end(), ends.first);
  ASSERT_EQUAL_QUIET(result_val.end(), ends.second);
  ASSERT_EQUAL(ref_key, result_key);
  ASSERT_EQUAL(ref_val, result_val);

  cudaStreamDestroy(s);
}
DECLARE_UNITTEST(TestMergeByKeyCudaStreams);


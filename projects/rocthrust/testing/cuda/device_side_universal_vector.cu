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

#include <thrust/universal_vector.h>

#include <unittest/unittest.h>

template <class VecT>
THRUST_HOST_DEVICE void universal_vector_access(VecT& in, thrust::universal_vector<bool>& out)
{
  const int expected_front = 4;
  const int expected_back  = 2;

  out[0] = in.size() == 2 &&               //
           in[0] == expected_front &&      //
           in.front() == expected_front && //
           *in.data() == expected_front && //
           in[1] == expected_back &&       //
           in.back() == expected_back;
}

#if defined(THRUST_TEST_DEVICE_SIDE)
template <class VecT>
__global__ void universal_vector_device_access_kernel(VecT &vec,
                                                      thrust::universal_vector<bool> &out)
{
  universal_vector_access(vec, out);
}

template <class VecT>
void test_universal_vector_access(VecT &vec, thrust::universal_vector<bool> &out)
{
  universal_vector_device_access_kernel<<<1, 1>>>(vec, out);
  cudaError_t const err = cudaDeviceSynchronize();
  ASSERT_EQUAL(cudaSuccess, err);
  ASSERT_EQUAL(out[0], true);
}
#else
template <class VecT>
void test_universal_vector_access(VecT &vec, thrust::universal_vector<bool> &out)
{
  universal_vector_access(vec, out);
  ASSERT_EQUAL(out[0], true);
}
#endif

void TestUniversalVectorDeviceAccess()
{
  using in_vector_t  = thrust::universal_vector<int>;
  using out_vector_t = thrust::universal_vector<bool>;

  in_vector_t *in_ptr{};
  cudaMallocManaged(&in_ptr, sizeof(*in_ptr));
  new (in_ptr) in_vector_t(1);

  auto &in = *in_ptr;
  in.resize(2);
  in[0] = 4;
  in[1] = 2;

  out_vector_t *out_ptr{};
  cudaMallocManaged(&out_ptr, sizeof(*out_ptr));
  new (out_ptr) out_vector_t(1);
  auto &out = *out_ptr;

  out.resize(1);
  out[0] = false;

  test_universal_vector_access(in, out);
}
DECLARE_UNITTEST(TestUniversalVectorDeviceAccess);

void TestConstUniversalVectorDeviceAccess()
{
  using in_vector_t  = thrust::universal_vector<int>;
  using out_vector_t = thrust::universal_vector<bool>;

  in_vector_t *in_ptr{};
  cudaMallocManaged(&in_ptr, sizeof(*in_ptr));
  new (in_ptr) in_vector_t(1);

  {
    auto &in = *in_ptr;
    in.resize(2);
    in[0] = 4;
    in[1] = 2;
  }

  const auto &const_in = *in_ptr;

  out_vector_t *out_ptr{};
  cudaMallocManaged(&out_ptr, sizeof(*out_ptr));
  new (out_ptr) out_vector_t(1);
  auto &out = *out_ptr;

  out.resize(1);
  out[0] = false;

  test_universal_vector_access(const_in, out);
}
DECLARE_UNITTEST(TestConstUniversalVectorDeviceAccess);

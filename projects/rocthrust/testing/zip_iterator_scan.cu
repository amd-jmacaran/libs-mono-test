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
#include <thrust/iterator/zip_iterator.h>
#include <thrust/scan.h>

#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA
#include <unittest/cuda/testframework.h>
#endif

using namespace unittest;


template<typename Tuple>
struct TuplePlus
{
  THRUST_HOST_DEVICE
  Tuple operator()(Tuple x, Tuple y) const
  {
    using namespace thrust;
    return make_tuple(get<0>(x) + get<0>(y),
                      get<1>(x) + get<1>(y));
  }
}; // end SumTuple


template <typename T>
struct TestZipIteratorScan
{
  void operator()(const size_t n)
  {
    using namespace thrust;

    host_vector<T> h_data0 = unittest::random_samples<T>(n);
    host_vector<T> h_data1 = unittest::random_samples<T>(n);

    device_vector<T> d_data0 = h_data0;
    device_vector<T> d_data1 = h_data1;

    using Tuple = tuple<T, T>;

    host_vector<Tuple>   h_result(n);
    device_vector<Tuple> d_result(n);

    // inclusive_scan (tuple output)
    inclusive_scan( make_zip_iterator(make_tuple(h_data0.begin(), h_data1.begin())),
                    make_zip_iterator(make_tuple(h_data0.end(),   h_data1.end())),
                    h_result.begin(),
                    TuplePlus<Tuple>());
    inclusive_scan( make_zip_iterator(make_tuple(d_data0.begin(), d_data1.begin())),
                    make_zip_iterator(make_tuple(d_data0.end(),   d_data1.end())),
                    d_result.begin(),
                    TuplePlus<Tuple>());
    ASSERT_EQUAL_QUIET(h_result, d_result);
   
    // exclusive_scan (tuple output)
    exclusive_scan( make_zip_iterator(make_tuple(h_data0.begin(), h_data1.begin())),
                    make_zip_iterator(make_tuple(h_data0.end(),   h_data1.end())),
                    h_result.begin(),
                    make_tuple<T,T>(0,0),
                    TuplePlus<Tuple>());
    exclusive_scan( make_zip_iterator(make_tuple(d_data0.begin(), d_data1.begin())),
                    make_zip_iterator(make_tuple(d_data0.end(),   d_data1.end())),
                    d_result.begin(),
                    make_tuple<T,T>(0,0),
                    TuplePlus<Tuple>());
    ASSERT_EQUAL_QUIET(h_result, d_result);

    host_vector<T>   h_result0(n);
    host_vector<T>   h_result1(n);
    device_vector<T> d_result0(n);
    device_vector<T> d_result1(n);
    
    // inclusive_scan (zip_iterator output)
    inclusive_scan( make_zip_iterator(make_tuple(h_data0.begin(), h_data1.begin())),
                    make_zip_iterator(make_tuple(h_data0.end(),   h_data1.end())),
                    make_zip_iterator(make_tuple(h_result0.begin(), h_result1.begin())),
                    TuplePlus<Tuple>());
    inclusive_scan( make_zip_iterator(make_tuple(d_data0.begin(), d_data1.begin())),
                    make_zip_iterator(make_tuple(d_data0.end(),   d_data1.end())),
                    make_zip_iterator(make_tuple(d_result0.begin(), d_result1.begin())),
                    TuplePlus<Tuple>());
    ASSERT_EQUAL_QUIET(h_result0, d_result0);
    ASSERT_EQUAL_QUIET(h_result1, d_result1);
    
    // exclusive_scan (zip_iterator output)
    exclusive_scan( make_zip_iterator(make_tuple(h_data0.begin(), h_data1.begin())),
                    make_zip_iterator(make_tuple(h_data0.end(),   h_data1.end())),
                    make_zip_iterator(make_tuple(h_result0.begin(), h_result1.begin())),
                    make_tuple<T,T>(0,0),
                    TuplePlus<Tuple>());
    exclusive_scan( make_zip_iterator(make_tuple(d_data0.begin(), d_data1.begin())),
                    make_zip_iterator(make_tuple(d_data0.end(),   d_data1.end())),
                    make_zip_iterator(make_tuple(d_result0.begin(), d_result1.begin())),
                    make_tuple<T,T>(0,0),
                    TuplePlus<Tuple>());
    ASSERT_EQUAL_QUIET(h_result0, d_result0);
    ASSERT_EQUAL_QUIET(h_result1, d_result1);
  }
};
VariableUnitTest<TestZipIteratorScan, SignedIntegralTypes> TestZipIteratorScanInstance;


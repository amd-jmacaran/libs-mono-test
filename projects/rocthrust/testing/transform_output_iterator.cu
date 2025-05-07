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

#include <thrust/copy.h>
#include <thrust/device_vector.h>
#include <thrust/functional.h>
#include <thrust/host_vector.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_output_iterator.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>

template <class Vector>
void TestTransformOutputIterator(void)
{
    using T = typename Vector::value_type;

    using UnaryFunction = thrust::square<T>;
    using Iterator = typename Vector::iterator;

    Vector input(4);
    Vector output(4);
    
    // initialize input
    thrust::sequence(input.begin(), input.end(), T{1});
   
    // construct transform_iterator
    thrust::transform_output_iterator<UnaryFunction, Iterator> output_iter(output.begin(), UnaryFunction());

    thrust::copy(input.begin(), input.end(), output_iter);

    Vector gold_output(4);
    gold_output[0] = 1;
    gold_output[1] = 4;
    gold_output[2] = 9;
    gold_output[3] = 16;

    ASSERT_EQUAL(output, gold_output);

}
DECLARE_VECTOR_UNITTEST(TestTransformOutputIterator);

template <class Vector>
void TestMakeTransformOutputIterator(void)
{
    using T = typename Vector::value_type;

    using UnaryFunction = thrust::square<T>;

    Vector input(4);
    Vector output(4);
    
    // initialize input
    thrust::sequence(input.begin(), input.end(), 1);
   
    thrust::copy(input.begin(), input.end(),
                 thrust::make_transform_output_iterator(output.begin(), UnaryFunction()));

    Vector gold_output(4);
    gold_output[0] = 1;
    gold_output[1] = 4;
    gold_output[2] = 9;
    gold_output[3] = 16;
    ASSERT_EQUAL(output, gold_output);

}
DECLARE_VECTOR_UNITTEST(TestMakeTransformOutputIterator);

template <typename T>
struct TestTransformOutputIteratorScan
{
    void operator()(const size_t n)
    {
        thrust::host_vector<T>   h_data = unittest::random_samples<T>(n);
        thrust::device_vector<T> d_data = h_data;

        thrust::host_vector<T>   h_result(n);
        thrust::device_vector<T> d_result(n);

        // run on host
        thrust::inclusive_scan(thrust::make_transform_iterator(h_data.begin(), thrust::negate<T>()),
                               thrust::make_transform_iterator(h_data.end(),   thrust::negate<T>()),
                               h_result.begin());
        // run on device
        thrust::inclusive_scan(d_data.begin(), d_data.end(),
                               thrust::make_transform_output_iterator(
                                   d_result.begin(), thrust::negate<T>()));


        ASSERT_EQUAL(h_result, d_result);
    }
};
VariableUnitTest<TestTransformOutputIteratorScan, SignedIntegralTypes> TestTransformOutputIteratorScanInstance;


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

#include <thrust/sort.h>

#include "test_header.hpp"

TESTS_DEFINE(SortByKeyVariableTests, UnsignedIntegerTestsParams);

TYPED_TEST(SortByKeyVariableTests, TestSortVariableBits)
{
    using T = typename TestFixture::input_type;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(size_t num_bits = 0; num_bits < 8 * sizeof(T); num_bits += 3)
        {
            SCOPED_TRACE(testing::Message() << "with bits = " << size);

            for(auto seed : get_seeds())
            {
                SCOPED_TRACE(testing::Message() << "with seed= " << seed);

                thrust::host_vector<T> h_keys = get_random_data<T>(
                    size, get_default_limits<T>::min(), get_default_limits<T>::max(), seed);

                const T mask = (1 << num_bits) - 1;
                for(size_t i = 0; i < size; i++)
                    h_keys[i] &= mask;

                thrust::host_vector<T>   reference = h_keys;
                thrust::device_vector<T> d_keys    = h_keys;

                thrust::host_vector<T>   h_values = h_keys;
                thrust::device_vector<T> d_values = d_keys;

                std::sort(reference.begin(), reference.end());

                thrust::sort_by_key(h_keys.begin(), h_keys.end(), h_values.begin());
                thrust::sort_by_key(d_keys.begin(), d_keys.end(), d_values.begin());

                ASSERT_EQ(reference, h_keys);
                ASSERT_EQ(reference, h_values);

                ASSERT_EQ(h_keys, d_keys);
                ASSERT_EQ(h_values, d_values);
            }
        }
    }
}

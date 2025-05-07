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

#include <thrust/functional.h>
#include <thrust/iterator/retag.h>
#include <thrust/set_operations.h>
#include <thrust/sort.h>

#include "test_header.hpp"

TESTS_DEFINE(setDifferenceByKeyTests, FullTestsParams);
TESTS_DEFINE(SetDifferenceByKeyPrimitiveTests, NumericalTestsParams);

template <typename InputIterator1,
          typename InputIterator2,
          typename InputIterator3,
          typename InputIterator4,
          typename OutputIterator1,
          typename OutputIterator2>
thrust::pair<OutputIterator1, OutputIterator2> set_difference_by_key(my_system& system,
                                                                     InputIterator1,
                                                                     InputIterator1,
                                                                     InputIterator2,
                                                                     InputIterator2,
                                                                     InputIterator3,
                                                                     InputIterator4,
                                                                     OutputIterator1 keys_result,
                                                                     OutputIterator2 values_result)
{
    system.validate_dispatch();
    return thrust::make_pair(keys_result, values_result);
}

TEST(setDifferenceByKeyTests, TestSetDifferenceByKeyDispatchExplicit)
{
    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    thrust::device_vector<int> vec(1);

    my_system sys(0);
    thrust::set_difference_by_key(sys,
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin(),
                                  vec.begin());

    ASSERT_EQ(true, sys.is_valid());
}

template <typename InputIterator1,
          typename InputIterator2,
          typename InputIterator3,
          typename InputIterator4,
          typename OutputIterator1,
          typename OutputIterator2>
thrust::pair<OutputIterator1, OutputIterator2> set_difference_by_key(my_tag,
                                                                     InputIterator1,
                                                                     InputIterator1,
                                                                     InputIterator2,
                                                                     InputIterator2,
                                                                     InputIterator3,
                                                                     InputIterator4,
                                                                     OutputIterator1 keys_result,
                                                                     OutputIterator2 values_result)
{
    *keys_result = 13;
    return thrust::make_pair(keys_result, values_result);
}

TEST(setDifferenceByKeyTests, TestSetDifferenceByKeyDispatchImplicit)
{
    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    thrust::device_vector<int> vec(1);

    thrust::set_difference_by_key(thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()),
                                  thrust::retag<my_tag>(vec.begin()));

    ASSERT_EQ(13, vec.front());
}

TYPED_TEST(setDifferenceByKeyTests, TestSetDifferenceByKeySimple)
{
    using Vector   = typename TestFixture::input_type;
    using Iterator = typename Vector::iterator;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    Vector a_key(4), b_key(5);
    Vector a_val(4), b_val(5);

    a_key[0] = 0;
    a_key[1] = 2;
    a_key[2] = 4;
    a_key[3] = 5;
    a_val[0] = 0;
    a_val[1] = 0;
    a_val[2] = 0;
    a_val[3] = 0;

    b_key[0] = 0;
    b_key[1] = 3;
    b_key[2] = 3;
    b_key[3] = 4;
    b_key[4] = 6;
    b_val[0] = 1;
    b_val[1] = 1;
    b_val[2] = 1;
    b_val[3] = 1;
    b_val[4] = 1;

    Vector ref_key(2), ref_val(2);
    ref_key[0] = 2;
    ref_key[1] = 5;
    ref_val[0] = 0;
    ref_val[1] = 0;

    Vector result_key(2), result_val(2);

    thrust::pair<Iterator, Iterator> end = thrust::set_difference_by_key(a_key.begin(),
                                                                         a_key.end(),
                                                                         b_key.begin(),
                                                                         b_key.end(),
                                                                         a_val.begin(),
                                                                         b_val.begin(),
                                                                         result_key.begin(),
                                                                         result_val.begin());

    EXPECT_EQ(result_key.end(), end.first);
    EXPECT_EQ(result_val.end(), end.second);
    ASSERT_EQ(ref_key, result_key);
    ASSERT_EQ(ref_val, result_val);
}

TYPED_TEST(SetDifferenceByKeyPrimitiveTests, TestSetDifferenceByKey)
{
    using T = typename TestFixture::input_type;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<T> temp = get_random_data<unsigned short int>(
                size,
                std::numeric_limits<unsigned short int>::min(),
                std::numeric_limits<unsigned short int>::max(),
                seed);

            thrust::host_vector<T> random_keys = get_random_data<unsigned short int>(
                size,
                std::numeric_limits<unsigned short int>::min(),
                std::numeric_limits<unsigned short int>::max(),
                seed + seed_value_addition
            );
            thrust::host_vector<T> random_vals = get_random_data<unsigned short int>(
                size,
                std::numeric_limits<unsigned short int>::min(),
                std::numeric_limits<unsigned short int>::max(),
                seed + 2 * seed_value_addition
            );

            size_t denominators[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9};
            size_t num_denominators = sizeof(denominators) / sizeof(size_t);

            for(size_t i = 0; i < num_denominators; ++i)
            {
                size_t size_a = size / denominators[i];

                thrust::host_vector<T> h_a_keys(random_keys.begin(), random_keys.begin() + size_a);
                thrust::host_vector<T> h_b_keys(random_keys.begin() + size_a, random_keys.end());

                thrust::host_vector<T> h_a_vals(random_vals.begin(), random_vals.begin() + size_a);
                thrust::host_vector<T> h_b_vals(random_vals.begin() + size_a, random_vals.end());

                thrust::stable_sort(h_a_keys.begin(), h_a_keys.end());
                thrust::stable_sort(h_b_keys.begin(), h_b_keys.end());

                thrust::device_vector<T> d_a_keys = h_a_keys;
                thrust::device_vector<T> d_b_keys = h_b_keys;

                thrust::device_vector<T> d_a_vals = h_a_vals;
                thrust::device_vector<T> d_b_vals = h_b_vals;

                thrust::host_vector<T> h_result_keys(size);
                thrust::host_vector<T> h_result_vals(size);

                thrust::device_vector<T> d_result_keys(size);
                thrust::device_vector<T> d_result_vals(size);

                thrust::pair<typename thrust::host_vector<T>::iterator,
                             typename thrust::host_vector<T>::iterator>
                    h_end;

                thrust::pair<typename thrust::device_vector<T>::iterator,
                             typename thrust::device_vector<T>::iterator>
                    d_end;

                h_end = thrust::set_difference_by_key(h_a_keys.begin(),
                                                      h_a_keys.end(),
                                                      h_b_keys.begin(),
                                                      h_b_keys.end(),
                                                      h_a_vals.begin(),
                                                      h_b_vals.begin(),
                                                      h_result_keys.begin(),
                                                      h_result_vals.begin());
                h_result_keys.erase(h_end.first, h_result_keys.end());
                h_result_vals.erase(h_end.second, h_result_vals.end());

                d_end = thrust::set_difference_by_key(d_a_keys.begin(),
                                                      d_a_keys.end(),
                                                      d_b_keys.begin(),
                                                      d_b_keys.end(),
                                                      d_a_vals.begin(),
                                                      d_b_vals.begin(),
                                                      d_result_keys.begin(),
                                                      d_result_vals.begin());
                d_result_keys.erase(d_end.first, d_result_keys.end());
                d_result_vals.erase(d_end.second, d_result_vals.end());

                ASSERT_EQ(h_result_keys, d_result_keys);
                ASSERT_EQ(h_result_vals, d_result_vals);
            }
        }
    }
}

TYPED_TEST(SetDifferenceByKeyPrimitiveTests, TestSetDifferenceByKeyEquivalentRanges)
{
    using T = typename TestFixture::input_type;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<T> temp = get_random_data<T>(
                size, get_default_limits<T>::min(), get_default_limits<T>::max(), seed);

            thrust::host_vector<T> h_a_key = temp;
            thrust::sort(h_a_key.begin(), h_a_key.end());
            thrust::host_vector<T> h_b_key = h_a_key;

            thrust::host_vector<T> h_a_val = get_random_data<T>(
                size,
                get_default_limits<T>::min(),
                get_default_limits<T>::max(),
                seed + seed_value_addition
            );
            thrust::host_vector<T> h_b_val = get_random_data<T>(
                size,
                get_default_limits<T>::min(),
                get_default_limits<T>::max(),
                seed + 2 * seed_value_addition
            );

            thrust::device_vector<T> d_a_key = h_a_key;
            thrust::device_vector<T> d_b_key = h_b_key;

            thrust::device_vector<T> d_a_val = h_a_val;
            thrust::device_vector<T> d_b_val = h_b_val;

            thrust::host_vector<T>   h_result_key(size), h_result_val(size);
            thrust::device_vector<T> d_result_key(size), d_result_val(size);

            thrust::pair<typename thrust::host_vector<T>::iterator,
                         typename thrust::host_vector<T>::iterator>
                h_end;

            thrust::pair<typename thrust::device_vector<T>::iterator,
                         typename thrust::device_vector<T>::iterator>
                d_end;

            h_end = thrust::set_difference_by_key(h_a_key.begin(),
                                                  h_a_key.end(),
                                                  h_b_key.begin(),
                                                  h_b_key.end(),
                                                  h_a_val.begin(),
                                                  h_b_val.begin(),
                                                  h_result_key.begin(),
                                                  h_result_val.begin());
            h_result_key.erase(h_end.first, h_result_key.end());
            h_result_val.erase(h_end.second, h_result_val.end());

            d_end = thrust::set_difference_by_key(d_a_key.begin(),
                                                  d_a_key.end(),
                                                  d_b_key.begin(),
                                                  d_b_key.end(),
                                                  d_a_val.begin(),
                                                  d_b_val.begin(),
                                                  d_result_key.begin(),
                                                  d_result_val.begin());
            d_result_key.erase(d_end.first, d_result_key.end());
            d_result_val.erase(d_end.second, d_result_val.end());

            ASSERT_EQ(h_result_key, d_result_key);
            ASSERT_EQ(h_result_val, d_result_val);
        }
    }
}

TYPED_TEST(SetDifferenceByKeyPrimitiveTests, TestSetDifferenceByKeyMultiset)
{
    using T = typename TestFixture::input_type;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<T> temp = get_random_data<T>(
                2 * size, get_default_limits<T>::min(), get_default_limits<T>::max(), seed);

            // restrict elements to [min,13)
            for(typename thrust::host_vector<T>::iterator i = temp.begin(); i != temp.end(); ++i)
            {
                int temp = static_cast<int>(*i);
                temp %= 13;
                *i = temp;
            }

            thrust::host_vector<T> h_a_key(temp.begin(), temp.begin() + size);
            thrust::host_vector<T> h_b_key(temp.begin() + size, temp.end());

            thrust::sort(h_a_key.begin(), h_a_key.end());
            thrust::sort(h_b_key.begin(), h_b_key.end());

            thrust::host_vector<T> h_a_val = get_random_data<T>(
                size,
                get_default_limits<T>::min(),
                get_default_limits<T>::max(),
                seed + seed_value_addition
            );
            thrust::host_vector<T> h_b_val = get_random_data<T>(
                size,
                get_default_limits<T>::min(),
                get_default_limits<T>::max(),
                seed + 2 * seed_value_addition
            );

            thrust::device_vector<T> d_a_key = h_a_key;
            thrust::device_vector<T> d_b_key = h_b_key;

            thrust::device_vector<T> d_a_val = h_a_val;
            thrust::device_vector<T> d_b_val = h_b_val;

            thrust::host_vector<T>   h_result_key(size), h_result_val(size);
            thrust::device_vector<T> d_result_key(size), d_result_val(size);

            thrust::pair<typename thrust::host_vector<T>::iterator,
                         typename thrust::host_vector<T>::iterator>
                h_end;

            thrust::pair<typename thrust::device_vector<T>::iterator,
                         typename thrust::device_vector<T>::iterator>
                d_end;

            h_end = thrust::set_difference_by_key(h_a_key.begin(),
                                                  h_a_key.end(),
                                                  h_b_key.begin(),
                                                  h_b_key.end(),
                                                  h_a_val.begin(),
                                                  h_b_val.begin(),
                                                  h_result_key.begin(),
                                                  h_result_val.begin());
            h_result_key.erase(h_end.first, h_result_key.end());
            h_result_val.erase(h_end.second, h_result_val.end());

            d_end = thrust::set_difference_by_key(d_a_key.begin(),
                                                  d_a_key.end(),
                                                  d_b_key.begin(),
                                                  d_b_key.end(),
                                                  d_a_val.begin(),
                                                  d_b_val.begin(),
                                                  d_result_key.begin(),
                                                  d_result_val.begin());
            d_result_key.erase(d_end.first, d_result_key.end());
            d_result_val.erase(d_end.second, d_result_val.end());

            ASSERT_EQ(h_result_key, d_result_key);
            ASSERT_EQ(h_result_val, d_result_val);
        }
    }
}

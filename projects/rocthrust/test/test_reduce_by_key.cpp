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

#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/iterator/retag.h>
#include <thrust/reduce.h>
#include <thrust/unique.h>

#include "test_header.hpp"

TESTS_DEFINE(ReduceByKeysIntegralTests, IntegerTestsParams);
TESTS_DEFINE(ReduceByKeysTests, FullTestsParams);

template <typename T>
struct is_equal_div_10_reduce
{
    __host__ __device__ bool operator()(const T x, const T& y) const
    {
        return ((int)x / 10) == ((int)y / 10);
    }
};

template <typename Vector>
void initialize_keys(Vector& keys)
{
    keys.resize(9);
    keys[0] = 11;
    keys[1] = 11;
    keys[2] = 21;
    keys[3] = 20;
    keys[4] = 21;
    keys[5] = 21;
    keys[6] = 21;
    keys[7] = 37;
    keys[8] = 37;
}

template <typename Vector>
void initialize_values(Vector& values)
{
    values.resize(9);
    values[0] = 0;
    values[1] = 1;
    values[2] = 2;
    values[3] = 3;
    values[4] = 4;
    values[5] = 5;
    values[6] = 6;
    values[7] = 7;
    values[8] = 8;
}

TYPED_TEST(ReduceByKeysTests, TestReduceByKeySimple)
{
    using Vector = typename TestFixture::input_type;
    using Policy = typename TestFixture::execution_policy;
    using T      = typename Vector::value_type;

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    Vector keys;
    Vector values;

    typename thrust::pair<typename Vector::iterator, typename Vector::iterator> new_last;

    // basic test
    initialize_keys(keys);
    initialize_values(values);

    Vector output_keys(keys.size());
    Vector output_values(values.size());

    new_last = thrust::reduce_by_key(
        Policy{}, keys.begin(), keys.end(), values.begin(), output_keys.begin(), output_values.begin());

    ASSERT_EQ(new_last.first - output_keys.begin(), 5);
    ASSERT_EQ(new_last.second - output_values.begin(), 5);
    ASSERT_EQ(output_keys[0], 11);
    ASSERT_EQ(output_keys[1], 21);
    ASSERT_EQ(output_keys[2], 20);
    ASSERT_EQ(output_keys[3], 21);
    ASSERT_EQ(output_keys[4], 37);

    ASSERT_EQ(output_values[0], 1);
    ASSERT_EQ(output_values[1], 2);
    ASSERT_EQ(output_values[2], 3);
    ASSERT_EQ(output_values[3], 15);
    ASSERT_EQ(output_values[4], 15);

    // test BinaryPredicate
    initialize_keys(keys);
    initialize_values(values);

    new_last = thrust::reduce_by_key(Policy{},
                                     keys.begin(),
                                     keys.end(),
                                     values.begin(),
                                     output_keys.begin(),
                                     output_values.begin(),
                                     is_equal_div_10_reduce<T>());

    ASSERT_EQ(new_last.first - output_keys.begin(), 3);
    ASSERT_EQ(new_last.second - output_values.begin(), 3);
    ASSERT_EQ(output_keys[0], 11);
    ASSERT_EQ(output_keys[1], 21);
    ASSERT_EQ(output_keys[2], 37);

    ASSERT_EQ(output_values[0], 1);
    ASSERT_EQ(output_values[1], 20);
    ASSERT_EQ(output_values[2], 15);

    // test BinaryFunction
    initialize_keys(keys);
    initialize_values(values);

    new_last = thrust::reduce_by_key(Policy{},
                                     keys.begin(),
                                     keys.end(),
                                     values.begin(),
                                     output_keys.begin(),
                                     output_values.begin(),
                                     thrust::equal_to<T>(),
                                     thrust::plus<T>());

    ASSERT_EQ(new_last.first - output_keys.begin(), 5);
    ASSERT_EQ(new_last.second - output_values.begin(), 5);

    ASSERT_EQ(output_keys[0], 11);
    ASSERT_EQ(output_keys[1], 21);
    ASSERT_EQ(output_keys[2], 20);
    ASSERT_EQ(output_keys[3], 21);
    ASSERT_EQ(output_keys[4], 37);

    ASSERT_EQ(output_values[0], 1);
    ASSERT_EQ(output_values[1], 2);
    ASSERT_EQ(output_values[2], 3);
    ASSERT_EQ(output_values[3], 15);
    ASSERT_EQ(output_values[4], 15);
}

TYPED_TEST(ReduceByKeysIntegralTests, TestReduceByKey)
{
    using K = typename TestFixture::input_type; // key type
    using V = unsigned int; // value type

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<K> h_keys = get_random_data<bool>(
                size,
                std::numeric_limits<bool>::min(),
                std::numeric_limits<bool>::max(),
                seed
            );
            thrust::host_vector<V> h_vals = get_random_data<V>(
                size,
                get_default_limits<V>::min(),
                get_default_limits<V>::max(),
                seed + seed_value_addition
            );
            thrust::device_vector<K> d_keys = h_keys;
            thrust::device_vector<V> d_vals = h_vals;

            thrust::host_vector<K>   h_keys_output(size);
            thrust::host_vector<V>   h_vals_output(size);
            thrust::device_vector<K> d_keys_output(size);
            thrust::device_vector<V> d_vals_output(size);

            using HostKeyIterator   = typename thrust::host_vector<K>::iterator;
            using HostValIterator   = typename thrust::host_vector<V>::iterator;
            using DeviceKeyIterator = typename thrust::device_vector<K>::iterator;
            using DeviceValIterator = typename thrust::device_vector<V>::iterator;

            using HostIteratorPair   = typename thrust::pair<HostKeyIterator, HostValIterator>;
            using DeviceIteratorPair = typename thrust::pair<DeviceKeyIterator, DeviceValIterator>;

            HostIteratorPair   h_last = thrust::reduce_by_key(h_keys.begin(),
                                                            h_keys.end(),
                                                            h_vals.begin(),
                                                            h_keys_output.begin(),
                                                            h_vals_output.begin());
            DeviceIteratorPair d_last = thrust::reduce_by_key(d_keys.begin(),
                                                              d_keys.end(),
                                                              d_vals.begin(),
                                                              d_keys_output.begin(),
                                                              d_vals_output.begin());

            ASSERT_EQ(h_last.first - h_keys_output.begin(), d_last.first - d_keys_output.begin());
            ASSERT_EQ(h_last.second - h_vals_output.begin(), d_last.second - d_vals_output.begin());

            size_t N = h_last.first - h_keys_output.begin();

            h_keys_output.resize(N);
            h_vals_output.resize(N);
            d_keys_output.resize(N);
            d_vals_output.resize(N);

            ASSERT_EQ(h_keys_output, d_keys_output);
            ASSERT_EQ(h_vals_output, d_vals_output);
        }
    }
};

TYPED_TEST(ReduceByKeysIntegralTests, TestReduceByKeyToDiscardIterator)
{
    using V = typename TestFixture::input_type; // value type
    using K = unsigned int; // key type

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<K> h_keys = get_random_data<bool>(
                size,
                std::numeric_limits<bool>::min(),
                std::numeric_limits<bool>::max(),
                seed
            );
            thrust::host_vector<V> h_vals = get_random_data<V>(
                size,
                get_default_limits<V>::min(),
                get_default_limits<V>::max(),
                seed + seed_value_addition
            );
            thrust::device_vector<K> d_keys = h_keys;
            thrust::device_vector<V> d_vals = h_vals;

            thrust::host_vector<K>   h_keys_output(size);
            thrust::host_vector<V>   h_vals_output(size);
            thrust::device_vector<K> d_keys_output(size);
            thrust::device_vector<V> d_vals_output(size);

            thrust::host_vector<K> unique_keys = h_keys;
            unique_keys.erase(thrust::unique(unique_keys.begin(), unique_keys.end()),
                              unique_keys.end());

            // discard key output
            size_t h_size = thrust::reduce_by_key(h_keys.begin(),
                                                  h_keys.end(),
                                                  h_vals.begin(),
                                                  thrust::make_discard_iterator(),
                                                  h_vals_output.begin())
                                .second
                            - h_vals_output.begin();

            size_t d_size = thrust::reduce_by_key(d_keys.begin(),
                                                  d_keys.end(),
                                                  d_vals.begin(),
                                                  thrust::make_discard_iterator(),
                                                  d_vals_output.begin())
                                .second
                            - d_vals_output.begin();

            h_vals_output.resize(h_size);
            d_vals_output.resize(d_size);

            ASSERT_EQ(h_vals_output.size(), unique_keys.size());
            ASSERT_EQ(d_vals_output.size(), unique_keys.size());
            ASSERT_EQ(d_vals_output.size(), h_vals_output.size());
        }
    }
};

template <typename InputIterator1,
          typename InputIterator2,
          typename OutputIterator1,
          typename OutputIterator2>
thrust::pair<OutputIterator1, OutputIterator2> reduce_by_key(my_system& system,
                                                             InputIterator1,
                                                             InputIterator1,
                                                             InputIterator2,
                                                             OutputIterator1 keys_output,
                                                             OutputIterator2 values_output)
{
    system.validate_dispatch();
    return thrust::make_pair(keys_output, values_output);
}

TEST(ReduceByKeysTests, TestReduceByKeyDispatchExplicit)
{
    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    thrust::device_vector<int> vec(1);

    my_system sys(0);
    thrust::reduce_by_key(sys, vec.begin(), vec.begin(), vec.begin(), vec.begin(), vec.begin());

    ASSERT_EQ(true, sys.is_valid());
}

template <typename InputIterator1,
          typename InputIterator2,
          typename OutputIterator1,
          typename OutputIterator2>
thrust::pair<OutputIterator1, OutputIterator2> reduce_by_key(my_tag,
                                                             InputIterator1,
                                                             InputIterator1,
                                                             InputIterator2,
                                                             OutputIterator1 keys_output,
                                                             OutputIterator2 values_output)
{
    *keys_output = 13;
    return thrust::make_pair(keys_output, values_output);
}

TEST(ReduceByKeysTests, TestReduceByKeyDispatchImplicit)
{
    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    thrust::device_vector<int> vec(1);

    thrust::reduce_by_key(thrust::retag<my_tag>(vec.begin()),
                          thrust::retag<my_tag>(vec.begin()),
                          thrust::retag<my_tag>(vec.begin()),
                          thrust::retag<my_tag>(vec.begin()),
                          thrust::retag<my_tag>(vec.begin()));

    ASSERT_EQ(13, vec.front());
}


__global__
THRUST_HIP_LAUNCH_BOUNDS_DEFAULT
void ReduceByKeyKernel(int const N, int *in_keys, int * in_values, int * out_keys, int * out_values, int * out_sizes)
{
    if(threadIdx.x == 0)
    {
        thrust::device_ptr<int> in_keys_begin(in_keys);
        thrust::device_ptr<int> in_keys_end(in_keys + N);
        thrust::device_ptr<int> in_values_begin(in_values);
        thrust::device_ptr<int> out_keys_begin(out_keys);
        thrust::device_ptr<int> out_values_begin(out_values);

        auto end_pair = thrust::reduce_by_key(thrust::hip::par, in_keys_begin,in_keys_end,
                                                      in_values_begin,
                                                      out_keys_begin,
                                                      out_values_begin);
       out_sizes[0] = end_pair.first - out_keys_begin;
       out_sizes[1] = end_pair.second - out_values_begin;
    }
}


TEST(ReduceByKeyTests, TestReduceByKeyDevice)
{

    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    for(auto size : get_sizes())
    {
        SCOPED_TRACE(testing::Message() << "with size= " << size);

        for(auto seed : get_seeds())
        {
            SCOPED_TRACE(testing::Message() << "with seed= " << seed);

            thrust::host_vector<int> h_values = get_random_data<int>(
                size,
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(),
                seed);
            thrust::host_vector<int> h_keys = get_random_data<int>(
                size, 0, 13, seed);
            thrust::device_vector<int> d_values = h_values;
            thrust::device_vector<int> d_keys = h_keys;

            thrust::host_vector<int> h_keys_result(size);
            thrust::host_vector<int> h_values_result(size);

            thrust::device_vector<int> d_keys_result(size);
            thrust::device_vector<int> d_values_result(size);
            thrust::device_vector<int> d_output_sizes(2);




            auto end_pair = thrust::reduce_by_key(h_keys.begin(), h_keys.end(),
                                                 h_values.begin(),
                                                 h_keys_result.begin(),
                                                 h_values_result.begin());

            hipLaunchKernelGGL(ReduceByKeyKernel,
                               dim3(1, 1, 1),
                               dim3(128, 1, 1),
                               0,
                               0,
                               size,
                               thrust::raw_pointer_cast(&d_keys[0]),
                               thrust::raw_pointer_cast(&d_values[0]),
                               thrust::raw_pointer_cast(&d_keys_result[0]),
                               thrust::raw_pointer_cast(&d_values_result[0]),
                               thrust::raw_pointer_cast(&d_output_sizes[0]));

            h_keys_result.resize(end_pair.first - h_keys_result.begin());
            h_values_result.resize(end_pair.second - h_values_result.begin());
            d_keys_result.resize(d_output_sizes[0]);
            d_values_result.resize(d_output_sizes[1]);

            ASSERT_EQ(h_keys_result,d_keys_result);
            ASSERT_EQ(h_values_result,d_values_result);

        }
    }
}

// Maps indices to key ids
class div_op
{
    std::int64_t m_divisor;

public:
    __host__ div_op(std::int64_t divisor) : m_divisor(divisor)
    {}

    __host__ __device__
    std::int64_t operator()(std::int64_t x) const
    {
        return x / m_divisor;
    }
};

// Produces unique sequence for key
class mod_op
{
    std::int64_t m_divisor;

public:
    __host__ mod_op(std::int64_t divisor) : m_divisor(divisor)
    {}

    __host__ __device__
    std::int64_t operator()(std::int64_t x) const
    {
        // div: 2          
        // idx: 0 1   2 3   4 5 
        // key: 0 0 | 1 1 | 2 2 
        // mod: 0 1 | 0 1 | 0 1
        // ret: 0 1   1 2   2 3
        return (x % m_divisor) + (x / m_divisor);
    }
};

TEST(ReduceByKeyTests, TestReduceByKeyWithBigIndexes)
{
    SCOPED_TRACE(testing::Message() << "with device_id= " << test::set_device_from_ctest());

    std::vector<size_t> magnitudes = {
        30, 31, 32, 33
    };

    for(auto magnitude : magnitudes)
    {
        const std::int64_t num_items = 1ll << magnitude;

        SCOPED_TRACE(testing::Message() << "with size= " << num_items);

        const std::int64_t key_size_magnitude = 8;
        ASSERT_TRUE(key_size_magnitude < magnitude);

        const std::int64_t num_unique_keys = 1ll << key_size_magnitude;

        // Size of each key group
        const std::int64_t key_size = num_items / num_unique_keys;

        using counting_it      = thrust::counting_iterator<std::int64_t>;
        using transform_key_it = thrust::transform_iterator<div_op, counting_it>;
        using transform_val_it = thrust::transform_iterator<mod_op, counting_it>;

        counting_it count_begin(0ll);
        counting_it count_end = count_begin + num_items;
        ASSERT_EQ(static_cast<std::int64_t>(thrust::distance(count_begin, count_end)), num_items);

        transform_key_it keys_begin(count_begin, div_op{key_size});
        transform_key_it keys_end(count_end, div_op{key_size});

        transform_val_it values_begin(count_begin, mod_op{key_size});

        thrust::device_vector<std::int64_t> output_keys(num_unique_keys);
        thrust::device_vector<std::int64_t> output_values(num_unique_keys);

        // example:
        //  items:        6
        //  unique_keys:  2
        //  key_size:     3
        //  keys:         0 0 0 | 1 1 1 
        //  values:       0 1 2 | 1 2 3
        //  result:       3       6     = sum(range(key_size)) + key_size * key_id
        thrust::reduce_by_key(keys_begin,
                              keys_end,
                              values_begin,
                              output_keys.begin(),
                              output_values.begin());

        ASSERT_TRUE(thrust::equal(output_keys.begin(), output_keys.end(), count_begin));

        thrust::host_vector<std::int64_t> result = output_values;

        const std::int64_t sum = (key_size - 1) * key_size / 2;
        for (std::int64_t key_id = 0; key_id < num_unique_keys; key_id++)
        {
            ASSERT_EQ(result[key_id], sum + key_id * key_size);
        }
    }
}

// MIT License
//
// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "common_test_header.hpp"

// hipcub API
#include "hipcub/device/device_reduce.hpp"

#include "test_utils_data_generation.hpp"

template<class Key,
         class Value,
         class ReduceOp,
         unsigned int MinSegmentLength,
         unsigned int MaxSegmentLength,
         class Aggregate = Value,
         bool UseGraphs  = false>
struct params
{
    using key_type                                   = Key;
    using value_type                                 = Value;
    using reduce_op_type                             = ReduceOp;
    static constexpr unsigned int min_segment_length = MinSegmentLength;
    static constexpr unsigned int max_segment_length = MaxSegmentLength;
    using aggregate_type                             = Aggregate;
    static constexpr bool use_graphs                 = UseGraphs;
};

template<class Params>
class HipcubDeviceReduceByKey : public ::testing::Test {
public:
    using params = Params;
};

using Params = ::testing::Types<
    params<int, int, hipcub::Sum, 1, 1>,
    params<double, int, hipcub::Sum, 3, 5>,
    params<float, int, hipcub::Sum, 1, 10>,
    params<unsigned long long, float, hipcub::Min, 1, 30>,
    params<int, unsigned int, hipcub::Max, 20, 100, short>,
    params<float, unsigned long long, hipcub::Max, 100, 400>,
    params<unsigned int, unsigned int, hipcub::Sum, 200, 600>,
    params<double, int, hipcub::Sum, 100, 2000>,
    params<int, unsigned int, hipcub::Sum, 1000, 5000>,
    params<unsigned int, int, hipcub::Sum, 2048, 2048>,
    params<unsigned int, double, hipcub::Min, 1000, 50000>,
    params<long long, short, hipcub::Sum, 1000, 10000, long long>,
    params<unsigned long long, unsigned long long, hipcub::Sum, 100000, 100000>,
    // Sum for half and bfloat will result in values too big due to limited range.
    params<test_utils::half, test_utils::half, hipcub::Max, 3, 100>,
    params<test_utils::bfloat16, test_utils::bfloat16, hipcub::Max, 20, 100>,
    params<int, int, hipcub::Sum, 1, 1, int, true>>;

TYPED_TEST_SUITE(HipcubDeviceReduceByKey, Params);

TYPED_TEST(HipcubDeviceReduceByKey, ReduceByKey)
{
    int device_id = test_common_utils::obtain_device_from_ctest();
    SCOPED_TRACE(testing::Message() << "with device_id= " << device_id);
    HIP_CHECK(hipSetDevice(device_id));

    using key_type              = typename TestFixture::params::key_type;
    using native_key_type       = test_utils::convert_to_native_t<key_type>;
    using value_type            = typename TestFixture::params::value_type;
    using aggregate_type        = typename TestFixture::params::aggregate_type;
    using reduce_op_type        = typename TestFixture::params::reduce_op_type;
    using key_distribution_type = typename std::conditional<
        test_utils::is_floating_point<key_type>::value,
        std::uniform_real_distribution<test_utils::convert_to_fundamental_t<key_type>>,
        std::uniform_int_distribution<test_utils::convert_to_fundamental_t<key_type>>>::type;

    reduce_op_type reduce_op;
    hipcub::Equality key_compare_op;

    hipStream_t stream = 0; // default
    if(TestFixture::params::use_graphs)
    {
        // Default stream does not support hipGraph stream capture, so create one
        HIP_CHECK(hipStreamCreateWithFlags(&stream, hipStreamNonBlocking));
    }

    for(size_t seed_index = 0; seed_index < random_seeds_count + seed_size; seed_index++)
    {
        unsigned int seed_value
            = seed_index < random_seeds_count ? rand() : seeds[seed_index - random_seeds_count];
        SCOPED_TRACE(testing::Message() << "with seed= " << seed_value);

        for(size_t size : test_utils::get_sizes(seed_value))
        {
            SCOPED_TRACE(testing::Message() << "with size= " << size);

            // Generate data and calculate expected results
            std::vector<key_type> unique_expected;
            std::vector<aggregate_type> aggregates_expected;
            size_t unique_count_expected = 0;

            std::vector<key_type> keys_input(size);
            key_distribution_type key_delta_dis(1, 5);
            std::uniform_int_distribution<size_t> key_count_dis(
                TestFixture::params::min_segment_length,
                TestFixture::params::max_segment_length
            );
            std::vector<value_type> values_input = test_utils::get_random_data<value_type>(
                size,
                0,
                100,
                seed_value
            );

            size_t offset = 0;
            std::default_random_engine gen(seed_value + seed_value_addition);
            native_key_type            current_key
                = static_cast<native_key_type>(key_distribution_type(0, 100)(gen));
            native_key_type prev_key = current_key;
            while(offset < size)
            {
                const size_t key_count = key_count_dis(gen);
                current_key += key_delta_dis(gen);

                const size_t end = std::min(size, offset + key_count);
                for(size_t i = offset; i < end; i++)
                {
                    keys_input[i] = test_utils::convert_to_device<key_type>(current_key);
                }
                aggregate_type aggregate = values_input[offset];
                for(size_t i = offset + 1; i < end; i++)
                {
                    aggregate = reduce_op(aggregate, static_cast<aggregate_type>(values_input[i]));
                }

                // The first key of the segment must be written into unique
                // (it may differ from other keys in case of custom key compraison operators)
                if(unique_count_expected == 0 || !key_compare_op(prev_key, current_key))
                {
                    unique_expected.push_back(test_utils::convert_to_device<key_type>(current_key));
                    unique_count_expected++;
                    aggregates_expected.push_back(aggregate);
                }
                else
                {
                    aggregates_expected.back() = reduce_op(aggregates_expected.back(), aggregate);
                }

                prev_key = current_key;
                offset += key_count;
            }

            key_type * d_keys_input;
            value_type * d_values_input;
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_keys_input, size * sizeof(key_type)));
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_values_input, size * sizeof(value_type)));
            HIP_CHECK(
                hipMemcpy(
                    d_keys_input, keys_input.data(),
                    size * sizeof(key_type),
                    hipMemcpyHostToDevice
                )
            );
            HIP_CHECK(
                hipMemcpy(
                    d_values_input, values_input.data(),
                    size * sizeof(value_type),
                    hipMemcpyHostToDevice
                )
            );

            key_type * d_unique_output;
            aggregate_type * d_aggregates_output;
            unsigned int * d_unique_count_output;
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_unique_output, unique_count_expected * sizeof(key_type)));
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_aggregates_output, unique_count_expected * sizeof(aggregate_type)));
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_unique_count_output, sizeof(unsigned int)));

            size_t temporary_storage_bytes = 0;

            HIP_CHECK(hipcub::DeviceReduce::ReduceByKey(nullptr,
                                                        temporary_storage_bytes,
                                                        d_keys_input,
                                                        d_unique_output,
                                                        d_values_input,
                                                        d_aggregates_output,
                                                        d_unique_count_output,
                                                        reduce_op,
                                                        size,
                                                        stream));

            ASSERT_GT(temporary_storage_bytes, 0U);

            void * d_temporary_storage;
            HIP_CHECK(test_common_utils::hipMallocHelper(&d_temporary_storage, temporary_storage_bytes));

            test_utils::GraphHelper gHelper;
            if (TestFixture::params::use_graphs)
                gHelper.startStreamCapture(stream);

            HIP_CHECK(hipcub::DeviceReduce::ReduceByKey(d_temporary_storage,
                                                        temporary_storage_bytes,
                                                        d_keys_input,
                                                        d_unique_output,
                                                        d_values_input,
                                                        d_aggregates_output,
                                                        d_unique_count_output,
                                                        reduce_op,
                                                        size,
                                                        stream));

            if (TestFixture::params::use_graphs)
                gHelper.createAndLaunchGraph(stream);

            HIP_CHECK(hipFree(d_temporary_storage));

            std::vector<key_type> unique_output(unique_count_expected);
            std::vector<aggregate_type> aggregates_output(unique_count_expected);
            std::vector<unsigned int> unique_count_output(1);
            HIP_CHECK(
                hipMemcpy(
                    unique_output.data(), d_unique_output,
                    unique_count_expected * sizeof(key_type),
                    hipMemcpyDeviceToHost
                )
            );
            HIP_CHECK(
                hipMemcpy(
                    aggregates_output.data(), d_aggregates_output,
                    unique_count_expected * sizeof(aggregate_type),
                    hipMemcpyDeviceToHost
                )
            );
            HIP_CHECK(
                hipMemcpy(
                    unique_count_output.data(), d_unique_count_output,
                    sizeof(unsigned int),
                    hipMemcpyDeviceToHost
                )
            );

            HIP_CHECK(hipFree(d_keys_input));
            HIP_CHECK(hipFree(d_values_input));
            HIP_CHECK(hipFree(d_unique_output));
            HIP_CHECK(hipFree(d_aggregates_output));
            HIP_CHECK(hipFree(d_unique_count_output));

            ASSERT_EQ(unique_count_output[0], unique_count_expected);

            // Validating results
            ASSERT_NO_FATAL_FAILURE(test_utils::assert_eq(unique_output, unique_expected));
            ASSERT_NO_FATAL_FAILURE(
                test_utils::assert_near(aggregates_output,
                                        aggregates_expected,
                                        test_utils::precision<aggregate_type>::value
                                            * TestFixture::params::max_segment_length));

            if(TestFixture::params::use_graphs)
                gHelper.cleanupGraphHelper();
        }
    }

    if(TestFixture::params::use_graphs)
        HIP_CHECK(hipStreamDestroy(stream));
}

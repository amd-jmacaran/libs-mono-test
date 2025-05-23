// MIT License
//
// Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "common_benchmark_header.hpp"

// HIP API
#include "hipcub/block/block_adjacent_difference.hpp"

#include "hipcub/block/block_load.hpp"
#include "hipcub/block/block_store.hpp"

#ifndef DEFAULT_N
const size_t DEFAULT_N = 1024 * 1024 * 128;
#endif

template<class Benchmark,
         unsigned int BlockSize,
         unsigned int ItemsPerThread,
         bool         WithTile,
         typename... Args>
__global__ __launch_bounds__(BlockSize) void kernel(Args... args)
{
    Benchmark::template run<BlockSize, ItemsPerThread, WithTile>(args...);
}

template<class T>
struct minus
{
    HIPCUB_HOST_DEVICE inline constexpr T operator()(const T& a, const T& b) const
    {
        return a - b;
    }
};

struct subtract_left
{
    template<unsigned int BlockSize, unsigned int ItemsPerThread, bool WithTile, typename T>
    __device__ static void run(const T* d_input, T* d_output, unsigned int trials)
    {
        const unsigned int lid          = threadIdx.x;
        const unsigned int block_offset = blockIdx.x * ItemsPerThread * BlockSize;

        T input[ItemsPerThread];
        hipcub::LoadDirectStriped<BlockSize>(lid, d_input + block_offset, input);

        hipcub::BlockAdjacentDifference<T, BlockSize> adjacent_difference;

#pragma nounroll
        for(unsigned int trial = 0; trial < trials; trial++)
        {
            T output[ItemsPerThread];
            if(WithTile)
            {
                adjacent_difference.SubtractLeft(input, output, minus<T>{}, T(123));
            } else
            {
                adjacent_difference.SubtractLeft(input, output, minus<T>{});
            }

            for(unsigned int i = 0; i < ItemsPerThread; ++i)
            {
                input[i] += output[i];
            }

            __syncthreads();
        }

        hipcub::StoreDirectStriped<BlockSize>(lid, d_output + block_offset, input);
    }
};

struct subtract_left_partial_tile
{
    template<unsigned int BlockSize, unsigned int ItemsPerThread, bool WithTile, typename T>
    __device__ static void
        run(const T* d_input, const int* tile_sizes, T* d_output, unsigned int trials)
    {
        const unsigned int lid          = threadIdx.x;
        const unsigned int block_offset = blockIdx.x * ItemsPerThread * BlockSize;

        T input[ItemsPerThread];
        hipcub::LoadDirectStriped<BlockSize>(lid, d_input + block_offset, input);

        hipcub::BlockAdjacentDifference<T, BlockSize> adjacent_difference;

        int tile_size = tile_sizes[blockIdx.x];

        // Try to evenly distribute the length of tile_sizes between all the trials
        const auto tile_size_diff = (BlockSize * ItemsPerThread) / trials + 1;

#pragma nounroll
        for(unsigned int trial = 0; trial < trials; trial++)
        {
            T output[ItemsPerThread];

            if(WithTile)
            {
                adjacent_difference.SubtractLeftPartialTile(input,
                                                            output,
                                                            minus<T>{},
                                                            tile_size,
                                                            T(123));
            } else
            {
                adjacent_difference.SubtractLeftPartialTile(input, output, minus<T>{}, tile_size);
            }

            for(unsigned int i = 0; i < ItemsPerThread; ++i)
            {
                input[i] += output[i];
            }

            // Change the tile_size to even out the distribution
            tile_size = (tile_size + tile_size_diff) % (BlockSize * ItemsPerThread);
            __syncthreads();
        }

        hipcub::StoreDirectStriped<BlockSize>(lid, d_output + block_offset, input);
    }
};

struct subtract_right
{
    template<unsigned int BlockSize, unsigned int ItemsPerThread, bool WithTile, typename T>
    __device__ static void run(const T* d_input, T* d_output, unsigned int trials)
    {
        const unsigned int lid          = threadIdx.x;
        const unsigned int block_offset = blockIdx.x * ItemsPerThread * BlockSize;

        T input[ItemsPerThread];
        hipcub::LoadDirectStriped<BlockSize>(lid, d_input + block_offset, input);

        hipcub::BlockAdjacentDifference<T, BlockSize> adjacent_difference;

#pragma nounroll
        for(unsigned int trial = 0; trial < trials; trial++)
        {
            T output[ItemsPerThread];
            if(WithTile)
            {
                adjacent_difference.SubtractRight(input, output, minus<T>{}, T(123));
            } else
            {
                adjacent_difference.SubtractRight(input, output, minus<T>{});
            }

            for(unsigned int i = 0; i < ItemsPerThread; ++i)
            {
                input[i] += output[i];
            }

            __syncthreads();
        }

        hipcub::StoreDirectStriped<BlockSize>(lid, d_output + block_offset, input);
    }
};

struct subtract_right_partial_tile
{
    template<unsigned int BlockSize, unsigned int ItemsPerThread, bool WithTile, typename T>
    __device__ static void
        run(const T* d_input, const int* tile_sizes, T* d_output, unsigned int trials)
    {
        const unsigned int lid          = threadIdx.x;
        const unsigned int block_offset = blockIdx.x * ItemsPerThread * BlockSize;

        T input[ItemsPerThread];
        hipcub::LoadDirectStriped<BlockSize>(lid, d_input + block_offset, input);

        hipcub::BlockAdjacentDifference<T, BlockSize> adjacent_difference;

        int tile_size = tile_sizes[blockIdx.x];

        // Try to evenly distribute the length of tile_sizes between all the trials
        const auto tile_size_diff = (BlockSize * ItemsPerThread) / trials + 1;

#pragma nounroll
        for(unsigned int trial = 0; trial < trials; trial++)
        {
            T output[ItemsPerThread];

            adjacent_difference.SubtractRightPartialTile(input, output, minus<T>{}, tile_size);

            for(unsigned int i = 0; i < ItemsPerThread; ++i)
            {
                input[i] += output[i];
            }

            // Change the tile_size to even out the distribution
            tile_size = (tile_size + tile_size_diff) % (BlockSize * ItemsPerThread);
            __syncthreads();
        }

        hipcub::StoreDirectStriped<BlockSize>(lid, d_output + block_offset, input);
    }
};

template<class Benchmark,
         class T,
         unsigned int BlockSize,
         unsigned int ItemsPerThread,
         bool         WithTile,
         unsigned int Trials = 100>
auto run_benchmark(benchmark::State& state, hipStream_t stream, size_t N)
    -> std::enable_if_t<!std::is_same<Benchmark, subtract_left_partial_tile>::value
                        && !std::is_same<Benchmark, subtract_right_partial_tile>::value>
{
    constexpr auto items_per_block = BlockSize * ItemsPerThread;
    const auto     num_blocks      = (N + items_per_block - 1) / items_per_block;
    // Round up size to the next multiple of items_per_block
    const auto size = num_blocks * items_per_block;

    const std::vector<T> input = benchmark_utils::get_random_data<T>(size, T(0), T(10));
    T*                   d_input;
    T*                   d_output;
    HIP_CHECK(hipMalloc(&d_input, input.size() * sizeof(input[0])));
    HIP_CHECK(hipMalloc(&d_output, input.size() * sizeof(T)));
    HIP_CHECK(
        hipMemcpy(d_input, input.data(), input.size() * sizeof(input[0]), hipMemcpyHostToDevice));

    for(auto _ : state)
    {
        auto start = std::chrono::high_resolution_clock::now();

        hipLaunchKernelGGL(HIP_KERNEL_NAME(kernel<Benchmark, BlockSize, ItemsPerThread, WithTile>),
                           dim3(num_blocks),
                           dim3(BlockSize),
                           0,
                           stream,
                           d_input,
                           d_output,
                           Trials);
        HIP_CHECK(hipGetLastError());
        HIP_CHECK(hipDeviceSynchronize());

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds
            = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
    state.SetBytesProcessed(state.iterations() * Trials * size * sizeof(T));
    state.SetItemsProcessed(state.iterations() * Trials * size);

    HIP_CHECK(hipFree(d_input));
    HIP_CHECK(hipFree(d_output));
}

template<class Benchmark,
         class T,
         unsigned int BlockSize,
         unsigned int ItemsPerThread,
         bool         WithTile,
         unsigned int Trials = 100>
auto run_benchmark(benchmark::State& state, hipStream_t stream, size_t N)
    -> std::enable_if_t<std::is_same<Benchmark, subtract_left_partial_tile>::value
                        || std::is_same<Benchmark, subtract_right_partial_tile>::value>
{
    constexpr auto items_per_block = BlockSize * ItemsPerThread;
    const auto     num_blocks      = (N + items_per_block - 1) / items_per_block;
    // Round up size to the next multiple of items_per_block
    const auto size = num_blocks * items_per_block;

    const std::vector<T>   input = benchmark_utils::get_random_data<T>(size, T(0), T(10));
    const std::vector<int> tile_sizes
        = benchmark_utils::get_random_data<int>(num_blocks, 0, items_per_block);

    T*   d_input;
    int* d_tile_sizes;
    T*   d_output;
    HIP_CHECK(hipMalloc(&d_input, input.size() * sizeof(input[0])));
    HIP_CHECK(hipMalloc(&d_tile_sizes, tile_sizes.size() * sizeof(tile_sizes[0])));
    HIP_CHECK(hipMalloc(&d_output, input.size() * sizeof(T)));
    HIP_CHECK(
        hipMemcpy(d_input, input.data(), input.size() * sizeof(input[0]), hipMemcpyHostToDevice));
    HIP_CHECK(hipMemcpy(d_tile_sizes,
                        tile_sizes.data(),
                        tile_sizes.size() * sizeof(tile_sizes[0]),
                        hipMemcpyHostToDevice));

    for(auto _ : state)
    {
        auto start = std::chrono::high_resolution_clock::now();

        hipLaunchKernelGGL(HIP_KERNEL_NAME(kernel<Benchmark, BlockSize, ItemsPerThread, WithTile>),
                           dim3(num_blocks),
                           dim3(BlockSize),
                           0,
                           stream,
                           d_input,
                           d_tile_sizes,
                           d_output,
                           Trials);
        HIP_CHECK(hipGetLastError());
        HIP_CHECK(hipDeviceSynchronize());

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds
            = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
    state.SetBytesProcessed(state.iterations() * Trials * size * sizeof(T));
    state.SetItemsProcessed(state.iterations() * Trials * size);

    HIP_CHECK(hipFree(d_input));
    HIP_CHECK(hipFree(d_tile_sizes));
    HIP_CHECK(hipFree(d_output));
}

#define CREATE_BENCHMARK(T, BS, IPT, WITH_TILE)                                      \
    benchmark::RegisterBenchmark(                                                    \
        std::string("block_adjacent_difference<data_type:" #T ",block_size:" #BS     \
                    ">.sub_algorithm_name:"                                          \
                    + name + "<items_per_thread:" #IPT ",with_tile:" #WITH_TILE ">") \
            .c_str(),                                                                \
        &run_benchmark<Benchmark, T, BS, IPT, WITH_TILE>,                            \
        stream,                                                                      \
        size)

#define BENCHMARK_TYPE(type, block, with_tile)                                                    \
    CREATE_BENCHMARK(type, block, 1, with_tile), CREATE_BENCHMARK(type, block, 3, with_tile),     \
        CREATE_BENCHMARK(type, block, 4, with_tile), CREATE_BENCHMARK(type, block, 8, with_tile), \
        CREATE_BENCHMARK(type, block, 16, with_tile), CREATE_BENCHMARK(type, block, 32, with_tile)

template<class Benchmark>
void add_benchmarks(const std::string&                            name,
                    std::vector<benchmark::internal::Benchmark*>& benchmarks,
                    hipStream_t                                   stream,
                    size_t                                        size)
{
    std::vector<benchmark::internal::Benchmark*> bs = {BENCHMARK_TYPE(int, 256, false),
                                                       BENCHMARK_TYPE(float, 256, false),
                                                       BENCHMARK_TYPE(int8_t, 256, false),
                                                       BENCHMARK_TYPE(long long, 256, false),
                                                       BENCHMARK_TYPE(double, 256, false)};

    if(!std::is_same<Benchmark, subtract_right_partial_tile>::value)
    {
        bs.insert(bs.end(),
                  {BENCHMARK_TYPE(int, 256, true),
                   BENCHMARK_TYPE(float, 256, true),
                   BENCHMARK_TYPE(int8_t, 256, true),
                   BENCHMARK_TYPE(long long, 256, true),
                   BENCHMARK_TYPE(double, 256, true)});
    }

    benchmarks.insert(benchmarks.end(), bs.begin(), bs.end());
}

int main(int argc, char* argv[])
{
    cli::Parser parser(argc, argv);
    parser.set_optional<size_t>("size", "size", DEFAULT_N, "number of values");
    parser.set_optional<int>("trials", "trials", -1, "number of iterations");
    parser.run_and_exit_if_error();

    // Parse argv
    benchmark::Initialize(&argc, argv);
    const size_t size   = parser.get<size_t>("size");
    const int    trials = parser.get<int>("trials");

    // HIP
    hipStream_t     stream = 0; // default
    hipDeviceProp_t devProp;
    int             device_id = 0;
    HIP_CHECK(hipGetDevice(&device_id));
    HIP_CHECK(hipGetDeviceProperties(&devProp, device_id));

    std::cout << "benchmark_block_adjacent_difference" << std::endl;
    std::cout << "[HIP] Device name: " << devProp.name << std::endl;

    // Add benchmarks
    std::vector<benchmark::internal::Benchmark*> benchmarks;
    add_benchmarks<subtract_left>("subtract_left", benchmarks, stream, size);
    add_benchmarks<subtract_right>("subtract_right", benchmarks, stream, size);
    add_benchmarks<subtract_left_partial_tile>("subtract_left_partial_tile", benchmarks, stream, size);
    add_benchmarks<subtract_right_partial_tile>("subtract_right_partial_tile",
                                                benchmarks,
                                                stream,
                                                size);

    // Use manual timing
    for(auto& b : benchmarks)
    {
        b->UseManualTime();
        b->Unit(benchmark::kMillisecond);
    }

    // Force number of iterations
    if(trials > 0)
    {
        for(auto& b : benchmarks)
        {
            b->Iterations(trials);
        }
    }

    // Run benchmarks
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
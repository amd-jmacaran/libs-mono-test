/*
 *  Copyright 2019 NVIDIA Corporation
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

#pragma once

#include <thrust/detail/config.h>

THRUST_NAMESPACE_BEGIN
namespace cuda_cub {

namespace detail {

    template<typename Size>
    struct make_unsigned_special;

    template<>
    struct make_unsigned_special<int> { using type = unsigned int; };

    // this is special, because CUDA's atomicAdd doesn't have an overload
    // for unsigned long, for some godforsaken reason
    template<>
    struct make_unsigned_special<long> { using type = unsigned long long; };

    template<>
    struct make_unsigned_special<long long> { using type = unsigned long long; };

}
}
THRUST_NAMESPACE_END


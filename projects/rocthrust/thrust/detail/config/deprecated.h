/*
 *  Copyright 2018-2024 NVIDIA Corporation
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

/*! \file deprecated.h
 *  \brief Defines Thrust's deprecation macros
 */

#pragma once

#include <thrust/detail/config/compiler.h>
#include <thrust/detail/config/cpp_dialect.h>

#if defined(CUB_IGNORE_DEPRECATED_API) && !defined(THRUST_IGNORE_DEPRECATED_API)
#  define THRUST_IGNORE_DEPRECATED_API
#endif

#define THRUST_ALIAS_ATTRIBUTE(...)

#ifdef THRUST_IGNORE_DEPRECATED_API
#  define THRUST_DEPRECATED
#  define THRUST_DEPRECATED_BECAUSE(MSG)
#elif THRUST_CPP_DIALECT >= 2014
#  define THRUST_DEPRECATED [[deprecated]]
#  define THRUST_DEPRECATED_BECAUSE(MSG) [[deprecated(MSG)]]
#elif THRUST_HOST_COMPILER == THRUST_HOST_COMPILER_MSVC
#  define THRUST_DEPRECATED __declspec(deprecated)
#  define THRUST_DEPRECATED_BECAUSE(MSG) __declspec(deprecated(MSG))
#elif THRUST_HOST_COMPILER == THRUST_HOST_COMPILER_CLANG
#  define THRUST_DEPRECATED __attribute__((deprecated))
#  define THRUST_DEPRECATED_BECAUSE(MSG) __attribute__((deprecated(MSG)))
#elif THRUST_HOST_COMPILER == THRUST_HOST_COMPILER_GCC
#  define THRUST_DEPRECATED __attribute__((deprecated))
#  define THRUST_DEPRECATED_BECAUSE(MSG) __attribute__((deprecated(MSG)))
#else
#  define THRUST_DEPRECATED
#  define THRUST_DEPRECATED_BECAUSE(MSG)
#endif

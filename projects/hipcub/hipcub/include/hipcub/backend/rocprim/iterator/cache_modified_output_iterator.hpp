/******************************************************************************
 * Copyright (c) 2010-2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright (c) 2017-2025, Advanced Micro Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef HIPCUB_ROCPRIM_ITERATOR_CACHE_MODIFIED_OUTPUT_ITERATOR_HPP_
#define HIPCUB_ROCPRIM_ITERATOR_CACHE_MODIFIED_OUTPUT_ITERATOR_HPP_

#include "../../../config.hpp"

#include "../thread/thread_store.hpp"

#include "iterator_category.hpp"

#include <iostream>

BEGIN_HIPCUB_NAMESPACE

template <
    CacheStoreModifier  MODIFIER,
    typename            ValueType,
    typename            OffsetT = ptrdiff_t>
class CacheModifiedOutputIterator
{
private:

    // Proxy object
    struct Reference
    {
        ValueType* ptr;

        /// Constructor
        __host__ __device__ __forceinline__ Reference(ValueType* ptr) : ptr(ptr) {}

        /// Assignment
        __device__ __forceinline__ ValueType operator =(ValueType val)
        {
            ThreadStore<MODIFIER>(ptr, val);
            return val;
        }
    };

public:
    // Required iterator traits
    using self_type = CacheModifiedOutputIterator; ///< My own type
    using difference_type
        = OffsetT; ///< Type to express the result of subtracting one iterator from another
    using value_type = void; ///< The type of the element the iterator can point to
    using pointer    = void; ///< The type of a pointer to an element the iterator can point to
    using reference
        = Reference; ///< The type of a reference to an element the iterator can point to
    using iterator_category = typename detail::IteratorCategory<value_type, reference, false>::
        type; ///< The iterator category

private:

    ValueType* ptr;

public:

    /// Constructor
    template <typename QualifiedValueType>
    __host__ __device__ __forceinline__ CacheModifiedOutputIterator(
        QualifiedValueType* ptr)     ///< Native pointer to wrap
    :
        ptr(const_cast<typename std::remove_cv<QualifiedValueType>::type *>(ptr))
    {}

    /// Postfix increment
    __host__ __device__ __forceinline__ self_type operator++(int)
    {
        self_type retval = *this;
        ptr++;
        return retval;
    }


    /// Prefix increment
    __host__ __device__ __forceinline__ self_type operator++()
    {
        ptr++;
        return *this;
    }

    /// Indirection
    __host__ __device__ __forceinline__ reference operator*() const
    {
        return Reference(ptr);
    }

    /// Addition
    template <typename Distance>
    __host__ __device__ __forceinline__ self_type operator+(Distance n) const
    {
        self_type retval(ptr + n);
        return retval;
    }

    /// Addition assignment
    template <typename Distance>
    __host__ __device__ __forceinline__ self_type& operator+=(Distance n)
    {
        ptr += n;
        return *this;
    }

    /// Subtraction
    template <typename Distance>
    __host__ __device__ __forceinline__ self_type operator-(Distance n) const
    {
        self_type retval(ptr - n);
        return retval;
    }

    /// Subtraction assignment
    template <typename Distance>
    __host__ __device__ __forceinline__ self_type& operator-=(Distance n)
    {
        ptr -= n;
        return *this;
    }

    /// Distance
    __host__ __device__ __forceinline__ difference_type operator-(self_type other) const
    {
        return ptr - other.ptr;
    }

    /// Array subscript
    template <typename Distance>
    __host__ __device__ __forceinline__ reference operator[](Distance n) const
    {
        return Reference(ptr + n);
    }

    /// Equal to
    __host__ __device__ __forceinline__ bool operator==(const self_type& rhs) const
    {
        return (ptr == rhs.ptr);
    }

    /// Not equal to
    __host__ __device__ __forceinline__ bool operator!=(const self_type& rhs) const
    {
        return (ptr != rhs.ptr);
    }

#ifndef DOXYGEN_SHOULD_SKIP_THIS    // Do not document

    /// ostream operator
    friend std::ostream& operator<<(std::ostream& os, const self_type& itr)
    {
        (void)itr;
        return os;
    }

#endif
};

END_HIPCUB_NAMESPACE

#endif

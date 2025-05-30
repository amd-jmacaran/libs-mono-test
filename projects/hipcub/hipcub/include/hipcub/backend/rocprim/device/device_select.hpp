/******************************************************************************
 * Copyright (c) 2010-2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2018, NVIDIA CORPORATION.  All rights reserved.
 * Modifications Copyright (c) 2017-2024, Advanced Micro Devices, Inc.  All rights reserved.
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

#ifndef HIPCUB_ROCPRIM_DEVICE_DEVICE_SELECT_HPP_
#define HIPCUB_ROCPRIM_DEVICE_DEVICE_SELECT_HPP_

#include "../../../config.hpp"
#include "../../../util_deprecated.hpp"

#include "../thread/thread_operators.hpp"

#include <rocprim/device/device_select.hpp>

#include <type_traits>

BEGIN_HIPCUB_NAMESPACE

class DeviceSelect
{
public:
    template<typename InputIteratorT,
             typename FlagIterator,
             typename OutputIteratorT,
             typename NumSelectedIteratorT>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t Flagged(void*                d_temp_storage,
                              size_t&              temp_storage_bytes,
                              InputIteratorT       d_in,
                              FlagIterator         d_flags,
                              OutputIteratorT      d_out,
                              NumSelectedIteratorT d_num_selected_out,
                              int64_t              num_items,
                              hipStream_t          stream = 0)
    {
        return ::rocprim::select(d_temp_storage,
                                 temp_storage_bytes,
                                 d_in,
                                 d_flags,
                                 d_out,
                                 d_num_selected_out,
                                 num_items,
                                 stream,
                                 HIPCUB_DETAIL_DEBUG_SYNC_VALUE);
    }

    template<typename InputIteratorT,
             typename FlagIterator,
             typename OutputIteratorT,
             typename NumSelectedIteratorT>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t Flagged(void*                d_temp_storage,
                              size_t&              temp_storage_bytes,
                              InputIteratorT       d_in,
                              FlagIterator         d_flags,
                              OutputIteratorT      d_out,
                              NumSelectedIteratorT d_num_selected_out,
                              int64_t              num_items,
                              hipStream_t          stream,
                              bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return Flagged(d_temp_storage,
                       temp_storage_bytes,
                       d_in,
                       d_flags,
                       d_out,
                       d_num_selected_out,
                       num_items,
                       stream);
    }

    template<typename IteratorT, typename FlagIterator, typename NumSelectedIteratorT>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t Flagged(void*                d_temp_storage,
                              size_t&              temp_storage_bytes,
                              IteratorT            d_data,
                              FlagIterator         d_flags,
                              NumSelectedIteratorT d_num_selected_out,
                              int64_t              num_items,
                              hipStream_t          stream = 0)
    {
        return Flagged(d_temp_storage,
                       temp_storage_bytes,
                       d_data,
                       d_flags,
                       d_data,
                       d_num_selected_out,
                       num_items,
                       stream);
    }

    template<typename IteratorT, typename FlagIterator, typename NumSelectedIteratorT>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t Flagged(void*                d_temp_storage,
                              size_t&              temp_storage_bytes,
                              IteratorT            d_data,
                              FlagIterator         d_flags,
                              NumSelectedIteratorT d_num_selected_out,
                              int64_t              num_items,
                              hipStream_t          stream,
                              bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return Flagged(d_temp_storage,
                       temp_storage_bytes,
                       d_data,
                       d_flags,
                       d_num_selected_out,
                       num_items,
                       stream);
    }

    template<typename InputIteratorT,
             typename OutputIteratorT,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t If(void*                d_temp_storage,
                         size_t&              temp_storage_bytes,
                         InputIteratorT       d_in,
                         OutputIteratorT      d_out,
                         NumSelectedIteratorT d_num_selected_out,
                         int64_t              num_items,
                         SelectOp             select_op,
                         hipStream_t          stream = 0)
    {
        return ::rocprim::select(d_temp_storage,
                                 temp_storage_bytes,
                                 d_in,
                                 d_out,
                                 d_num_selected_out,
                                 num_items,
                                 select_op,
                                 stream,
                                 HIPCUB_DETAIL_DEBUG_SYNC_VALUE);
    }

    template<typename InputIteratorT,
             typename OutputIteratorT,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t If(void*                d_temp_storage,
                         size_t&              temp_storage_bytes,
                         InputIteratorT       d_in,
                         OutputIteratorT      d_out,
                         NumSelectedIteratorT d_num_selected_out,
                         int64_t              num_items,
                         SelectOp             select_op,
                         hipStream_t          stream,
                         bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return If(d_temp_storage,
                  temp_storage_bytes,
                  d_in,
                  d_out,
                  d_num_selected_out,
                  num_items,
                  select_op,
                  stream);
    }

    template<typename IteratorT, typename NumSelectedIteratorT, typename SelectOp>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t If(void*                d_temp_storage,
                         size_t&              temp_storage_bytes,
                         IteratorT            d_data,
                         NumSelectedIteratorT d_num_selected_out,
                         int64_t              num_items,
                         SelectOp             select_op,
                         hipStream_t          stream = 0)
    {
        return If(d_temp_storage,
                  temp_storage_bytes,
                  d_data,
                  d_data,
                  d_num_selected_out,
                  num_items,
                  select_op,
                  stream);
    }

    template<typename IteratorT, typename NumSelectedIteratorT, typename SelectOp>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t If(void*                d_temp_storage,
                         size_t&              temp_storage_bytes,
                         IteratorT            d_data,
                         NumSelectedIteratorT d_num_selected_out,
                         int64_t              num_items,
                         SelectOp             select_op,
                         hipStream_t          stream,
                         bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return If(d_temp_storage,
                  temp_storage_bytes,
                  d_data,
                  d_num_selected_out,
                  num_items,
                  select_op,
                  stream);
    }

    template<typename InputIteratorT,
             typename FlagIterator,
             typename OutputIteratorT,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t FlaggedIf(void*                d_temp_storage,
                                size_t&              temp_storage_bytes,
                                InputIteratorT       d_in,
                                FlagIterator         d_flags,
                                OutputIteratorT      d_out,
                                NumSelectedIteratorT d_num_selected_out,
                                int64_t              num_items,
                                SelectOp             select_op,
                                hipStream_t          stream = 0)
    {
        return ::rocprim::select(d_temp_storage,
                                 temp_storage_bytes,
                                 d_in,
                                 d_flags,
                                 d_out,
                                 d_num_selected_out,
                                 num_items,
                                 select_op,
                                 stream,
                                 HIPCUB_DETAIL_DEBUG_SYNC_VALUE);
    }

    template<typename InputIteratorT,
             typename FlagIterator,
             typename OutputIteratorT,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t FlaggedIf(void*                d_temp_storage,
                                size_t&              temp_storage_bytes,
                                InputIteratorT       d_in,
                                FlagIterator         d_flags,
                                OutputIteratorT      d_out,
                                NumSelectedIteratorT d_num_selected_out,
                                int64_t              num_items,
                                SelectOp             select_op,
                                hipStream_t          stream,
                                bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return FlaggedIf(d_temp_storage,
                         temp_storage_bytes,
                         d_in,
                         d_flags,
                         d_out,
                         d_num_selected_out,
                         num_items,
                         select_op,
                         stream);
    }

    template<typename IteratorT,
             typename FlagIterator,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t FlaggedIf(void*                d_temp_storage,
                                size_t&              temp_storage_bytes,
                                IteratorT            d_data,
                                FlagIterator         d_flags,
                                NumSelectedIteratorT d_num_selected_out,
                                int64_t              num_items,
                                SelectOp             select_op,
                                hipStream_t          stream = 0)
    {
        return FlaggedIf(d_temp_storage,
                         temp_storage_bytes,
                         d_data,
                         d_flags,
                         d_data,
                         d_num_selected_out,
                         num_items,
                         select_op,
                         stream);
    }

    template<typename IteratorT,
             typename FlagIterator,
             typename NumSelectedIteratorT,
             typename SelectOp>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t FlaggedIf(void*                d_temp_storage,
                                size_t&              temp_storage_bytes,
                                IteratorT            d_data,
                                FlagIterator         d_flags,
                                NumSelectedIteratorT d_num_selected_out,
                                int64_t              num_items,
                                SelectOp             select_op,
                                hipStream_t          stream,
                                bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return FlaggedIf(d_temp_storage,
                         temp_storage_bytes,
                         d_data,
                         d_flags,
                         d_num_selected_out,
                         num_items,
                         select_op,
                         stream);
    }

    template<typename InputIteratorT, typename OutputIteratorT, typename NumSelectedIteratorT>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t Unique(void*                d_temp_storage,
                             size_t&              temp_storage_bytes,
                             InputIteratorT       d_in,
                             OutputIteratorT      d_out,
                             NumSelectedIteratorT d_num_selected_out,
                             int64_t              num_items,
                             hipStream_t          stream = 0)
    {
        return ::rocprim::unique(d_temp_storage,
                                 temp_storage_bytes,
                                 d_in,
                                 d_out,
                                 d_num_selected_out,
                                 num_items,
                                 hipcub::Equality(),
                                 stream,
                                 HIPCUB_DETAIL_DEBUG_SYNC_VALUE);
    }

    template<typename InputIteratorT, typename OutputIteratorT, typename NumSelectedIteratorT>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t Unique(void*                d_temp_storage,
                             size_t&              temp_storage_bytes,
                             InputIteratorT       d_in,
                             OutputIteratorT      d_out,
                             NumSelectedIteratorT d_num_selected_out,
                             int64_t              num_items,
                             hipStream_t          stream,
                             bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return Unique(d_temp_storage,
                      temp_storage_bytes,
                      d_in,
                      d_out,
                      d_num_selected_out,
                      num_items,
                      stream);
    }

    template<typename KeyIteratorT,
             typename ValueIteratorT,
             typename OutputKeyIteratorT,
             typename OutputValueIteratorT,
             typename NumSelectedIteratorT,
             typename NumItemsT,
             typename EqualityOpT>
    HIPCUB_RUNTIME_FUNCTION
    static
        typename std::enable_if_t<!std::is_convertible<EqualityOpT, hipStream_t>::value, hipError_t>
        UniqueByKey(void*                d_temp_storage,
                    size_t&              temp_storage_bytes,
                    KeyIteratorT         d_keys_input,
                    ValueIteratorT       d_values_input,
                    OutputKeyIteratorT   d_keys_output,
                    OutputValueIteratorT d_values_output,
                    NumSelectedIteratorT d_num_selected_out,
                    NumItemsT            num_items,
                    EqualityOpT          equality_op,
                    hipStream_t          stream = 0)
    {
        return ::rocprim::unique_by_key(d_temp_storage,
                                        temp_storage_bytes,
                                        d_keys_input,
                                        d_values_input,
                                        d_keys_output,
                                        d_values_output,
                                        d_num_selected_out,
                                        num_items,
                                        equality_op,
                                        stream,
                                        HIPCUB_DETAIL_DEBUG_SYNC_VALUE);
    }

    template<typename KeyIteratorT,
             typename ValueIteratorT,
             typename OutputKeyIteratorT,
             typename OutputValueIteratorT,
             typename NumSelectedIteratorT,
             typename NumItemsT>
    HIPCUB_RUNTIME_FUNCTION
    static hipError_t UniqueByKey(void*                d_temp_storage,
                                  size_t&              temp_storage_bytes,
                                  KeyIteratorT         d_keys_input,
                                  ValueIteratorT       d_values_input,
                                  OutputKeyIteratorT   d_keys_output,
                                  OutputValueIteratorT d_values_output,
                                  NumSelectedIteratorT d_num_selected_out,
                                  NumItemsT            num_items,
                                  hipStream_t          stream = 0)
    {
        return UniqueByKey(d_temp_storage,
                           temp_storage_bytes,
                           d_keys_input,
                           d_values_input,
                           d_keys_output,
                           d_values_output,
                           d_num_selected_out,
                           num_items,
                           hipcub::Equality(),
                           stream);
    }

    template<typename KeyIteratorT,
             typename ValueIteratorT,
             typename OutputKeyIteratorT,
             typename OutputValueIteratorT,
             typename NumSelectedIteratorT,
             typename NumItemsT>
    HIPCUB_DETAIL_DEPRECATED_DEBUG_SYNCHRONOUS HIPCUB_RUNTIME_FUNCTION
    static hipError_t UniqueByKey(void*                d_temp_storage,
                                  size_t&              temp_storage_bytes,
                                  KeyIteratorT         d_keys_input,
                                  ValueIteratorT       d_values_input,
                                  OutputKeyIteratorT   d_keys_output,
                                  OutputValueIteratorT d_values_output,
                                  NumSelectedIteratorT d_num_selected_out,
                                  NumItemsT            num_items,
                                  hipStream_t          stream,
                                  bool                 debug_synchronous)
    {
        HIPCUB_DETAIL_RUNTIME_LOG_DEBUG_SYNCHRONOUS();
        return UniqueByKey(d_temp_storage,
                           temp_storage_bytes,
                           d_keys_input,
                           d_values_input,
                           d_keys_output,
                           d_values_output,
                           d_num_selected_out,
                           num_items,
                           stream);
    }
};

END_HIPCUB_NAMESPACE

#endif // HIPCUB_ROCPRIM_DEVICE_DEVICE_SELECT_HPP_

# ########################################################################
# Copyright 2020-2025 Advanced Micro Devices, Inc.
# ########################################################################

if (WIN32)
  set(DLLS_COPIED OFF)
endif()

function(add_thrust_example EXAMPLE)
    set(EXAMPLE_SOURCE "${EXAMPLE}.cu")
    set(EXAMPLE_TARGET "example_thrust_${EXAMPLE}")
    if(USE_HIPCXX)
      set_source_files_properties(${EXAMPLE_SOURCE}
        PROPERTIES
          LANGUAGE HIP
      )
    else()
      set_source_files_properties(${EXAMPLE_SOURCE}
          PROPERTIES
              LANGUAGE CXX
      )
    endif()
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_SOURCE})

    target_compile_options(${EXAMPLE_TARGET} PRIVATE ${COMPILE_OPTIONS})
    target_link_libraries(${EXAMPLE_TARGET}
        PRIVATE
            rocthrust
            roc::rocprim_hip
    )
    if (NOT WIN32)
      foreach(gpu_target ${GPU_TARGETS})
          target_link_libraries(${EXAMPLE_TARGET}
              INTERFACE
                  --cuda-gpu-arch=${gpu_target}
          )
      endforeach()
    endif()
    set_target_properties(${EXAMPLE_TARGET}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/"
    )
    if (WIN32 AND NOT DEFINED DLLS_COPIED)
      set(DLLS_COPIED "YES")
      set(DLLS_COPIED ${DLLS_COPIED} PARENT_SCOPE)
      # for now adding in all .dll as dependency chain is not cmake based on win32
      file( GLOB third_party_dlls
      LIST_DIRECTORIES ON
      CONFIGURE_DEPENDS
      ${HIP_DIR}/bin/*.dll
      ${CMAKE_SOURCE_DIR}/rtest.*
      )
      foreach( file_i ${third_party_dlls})
        add_custom_command( TARGET ${EXAMPLE_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${file_i} ${PROJECT_BINARY_DIR}/examples )
      endforeach( file_i )
    endif()
endfunction()


add_subdirectory(cpp_integration)

# ****************************************************************************
# Examples
# ****************************************************************************
message (STATUS "Building examples")

add_thrust_example("arbitrary_transformation")
add_thrust_example("basic_vector")
add_thrust_example("bounding_box")
# add_thrust_example("bucket_sort2d")
add_thrust_example("constant_iterator")
add_thrust_example("counting_iterator")
add_thrust_example("device_ptr")
add_thrust_example("discrete_voronoi")
add_thrust_example("dot_products_with_zip")
add_thrust_example("expand")
add_thrust_example("fill_copy_sequence")
add_thrust_example("histogram")
add_thrust_example("lambda")
add_thrust_example("lexicographical_sort")
add_thrust_example("max_abs_diff")
add_thrust_example("minimal_custom_backend")
add_thrust_example("minmax")
add_thrust_example("mode")
add_thrust_example("monte_carlo_disjoint_sequences")
add_thrust_example("monte_carlo")
add_thrust_example("norm")
add_thrust_example("padded_grid_reduction")
add_thrust_example("permutation_iterator")
add_thrust_example("raw_reference_cast")
add_thrust_example("remove_points2d")
add_thrust_example("repeated_range")
add_thrust_example("run_length_decoding")
add_thrust_example("run_length_encoding")
add_thrust_example("saxpy")
add_thrust_example("scan_by_key")
add_thrust_example("set_operations")
add_thrust_example("simple_moving_average")
add_thrust_example("sort")
add_thrust_example("sorting_aos_vs_soa")
add_thrust_example("sparse_vector")
add_thrust_example("stream_compaction")
add_thrust_example("strided_range")
add_thrust_example("sum_rows")
add_thrust_example("sum")
add_thrust_example("summary_statistics")
add_thrust_example("summed_area_table")
add_thrust_example("tiled_range")
add_thrust_example("transform_iterator")
add_thrust_example("transform_input_output_iterator")
add_thrust_example("transform_output_iterator")
add_thrust_example("uninitialized_vector")
add_thrust_example("version")
add_thrust_example("scan_matrix_by_rows")
# add_thrust_example("weld_vertices") // this example does not work yet.
add_thrust_example("word_count")

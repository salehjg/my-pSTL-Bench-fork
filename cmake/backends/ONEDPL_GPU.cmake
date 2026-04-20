if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND
        NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")
    message(WARNING "To use the ONEDPL_GPU backend, make sure you use an Intel compiler. Your compiler ID: ${CMAKE_CXX_COMPILER_ID}")
endif()

add_compile_definitions(PSTL_BENCH_USE_GPU)
add_compile_definitions(PSTL_BENCH_USE_ONEDPL)

add_compile_definitions(PSTL_BENCH_BACKEND="ONEDPL_GPU")

# USM shared memory: allocate data with sycl::malloc_shared so oneDPL
# avoids creating temporary SYCL buffers on every algorithm call.
option(PSTL_BENCH_ONEDPL_GPU_USM "Use SYCL USM shared memory instead of host iterators (ONEDPL_GPU only)" OFF)
if (PSTL_BENCH_ONEDPL_GPU_USM)
    add_compile_definitions(PSTL_BENCH_ONEDPL_GPU_USM)
    message(STATUS "ONEDPL_GPU: USM shared memory ENABLED")
else ()
    message(STATUS "ONEDPL_GPU: USM shared memory DISABLED (host iterators)")
endif ()

# Find Intel oneAPI DPC++ Library (oneDPL)
find_package(oneDPL REQUIRED)

if (NOT TARGET oneDPL)
    message(FATAL_ERROR "oneDPL target not found")
else ()
    message(STATUS "oneDPL target found")
    list(APPEND LINK_LIBRARIES oneDPL)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsycl")
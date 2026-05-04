if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND
        NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")
    message(WARNING "To use the ONEDPL_GPU backend, make sure you use an Intel compiler. Your compiler ID: ${CMAKE_CXX_COMPILER_ID}")
endif()

add_compile_definitions(PSTL_BENCH_USE_GPU)
add_compile_definitions(PSTL_BENCH_USE_ONEDPL)

add_compile_definitions(PSTL_BENCH_BACKEND="ONEDPL_GPU")

# Memory model — must be set explicitly. No default.
#   USM     -> sycl::malloc_shared (host-addressable, auto-migrated)
#   NO_USM  -> persistent sycl::buffer over the host vector
# See docs/notes/onedpl_gpu_fix_plan.md.
set(PSTL_BENCH_ONEDPL_GPU_MEMORY "" CACHE STRING
        "ONEDPL_GPU memory model: USM | NO_USM (must be set explicitly)")
set_property(CACHE PSTL_BENCH_ONEDPL_GPU_MEMORY PROPERTY STRINGS USM NO_USM)

if (PSTL_BENCH_ONEDPL_GPU_MEMORY STREQUAL "USM")
    add_compile_definitions(PSTL_BENCH_ONEDPL_GPU_USM)
    message(STATUS "ONEDPL_GPU: memory model = USM (sycl::malloc_shared)")
elseif (PSTL_BENCH_ONEDPL_GPU_MEMORY STREQUAL "NO_USM")
    add_compile_definitions(PSTL_BENCH_ONEDPL_GPU_NO_USM)
    message(STATUS "ONEDPL_GPU: memory model = NO_USM (persistent sycl::buffer)")
else ()
    message(FATAL_ERROR
            "PSTL_BENCH_ONEDPL_GPU_MEMORY must be set to USM or NO_USM "
            "(got: '${PSTL_BENCH_ONEDPL_GPU_MEMORY}'). "
            "Pass -DPSTL_BENCH_ONEDPL_GPU_MEMORY=USM or =NO_USM at configure time.")
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

# TBB for host-side parallel data preparation (host_parallel.h).
# oneDPL depends on TBB transitively, but finding it explicitly
# ensures the TBB headers are on the include path for our code.
find_package(TBB REQUIRED)
if (NOT TARGET TBB::tbb)
    message(FATAL_ERROR "ONEDPL_GPU backend needs TBB (for host-side parallel setup). TBB::tbb not found.")
endif ()
message(STATUS "ONEDPL_GPU: linking TBB::tbb (for host-side parallel data preparation)")
list(APPEND LINK_LIBRARIES TBB::tbb)
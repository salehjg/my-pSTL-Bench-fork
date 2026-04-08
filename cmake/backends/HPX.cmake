add_compile_definitions(PSTL_BENCH_USE_HPX)

add_compile_definitions(PSTL_BENCH_BACKEND="HPX")

# Find package HPX
find_package(HPX REQUIRED)
if (NOT TARGET HPX::hpx)
    message(FATAL_ERROR "HPX not found!")
else ()
    message(STATUS "HPX found")
    list(APPEND LINK_LIBRARIES HPX::hpx HPX::wrap_main)
endif ()

# If HPX was built with SYCL support but the current compiler doesn't support
# -fsycl (e.g. g++, nvc++, or vanilla clang++), strip the flags that HPX
# propagates via HPXInternal::hpx_async_sycl.
if (TARGET HPXInternal::hpx_async_sycl)
    get_target_property(_sycl_opts HPXInternal::hpx_async_sycl INTERFACE_COMPILE_OPTIONS)
    if (_sycl_opts)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-fsycl" _HAVE_FSYCL)
        if (NOT _HAVE_FSYCL)
            message(STATUS "Stripping SYCL flags from HPXInternal::hpx_async_sycl (compiler does not support -fsycl)")
            set_target_properties(HPXInternal::hpx_async_sycl PROPERTIES
                INTERFACE_COMPILE_OPTIONS ""
                INTERFACE_LINK_OPTIONS ""
            )
        endif ()
    endif ()
endif ()

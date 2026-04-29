# AdaptiveCpp (acpp) hosting oneDPL.
#
# Same source path as the existing ONEDPL_GPU backend (the *_onedpl.h
# headers that call oneapi::dpl::execution::*), but compiled with
# acpp's SYCL implementation instead of icpx's. Tests "is acpp's SYCL
# runtime a viable substrate for oneDPL?" rather than "is acpp's
# stdpar transformation correct?" (the ACPP backend covers the latter).
#
# We deliberately do NOT use --acpp-stdpar here: the *_onedpl.h path
# uses oneapi::dpl::execution::*, not std::execution::*, so there's
# nothing for stdpar's interceptor to do, and skipping stdpar means
# no allocator replacement — libbenchmark.a does not need to be
# rebuilt with the same flags as in the ACPP backend.
#
# See docs/notes/acpp_stdpar_sort_template_bug.md ("Related: oneDPL
# is an orthogonal path …") for the rationale and how this differs
# from the ACPP backend.

if (NOT CMAKE_CXX_COMPILER MATCHES "acpp$")
    message(WARNING "To use the ACPP_ONEDPL backend, set CMAKE_CXX_COMPILER=acpp. Got: ${CMAKE_CXX_COMPILER}")
endif ()

add_compile_definitions(PSTL_BENCH_USE_GPU)
add_compile_definitions(PSTL_BENCH_USE_ONEDPL)
add_compile_definitions(PSTL_BENCH_BACKEND="ACPP_ONEDPL")

# Enable the std::_Exit fast-path at end of main (see src/main.cpp). acpp
# 25.10 + oneDPL has an upstream bug in `hipsycl::common::allocation_map::erase`
# that fires during __cxa_finalize at process exit — the benchmark itself
# completes, the .done sentinel and JSON are flushed by benchmark::Shutdown()
# before the crash, but the kernel reports SIGSEGV/SIGBUS and we get
# exit 134/135/139. _Exit skips __cxa_finalize entirely and the process
# exits 0 cleanly. Other backends keep their normal shutdown semantics.
# Diagnosis: misc/acpp_onedpl_repro/ ; analysis: docs/notes/acpp_stdpar_sort_template_bug.md.
add_compile_definitions(PSTL_BENCH_QUICK_EXIT_AT_END)

# Memory model — same convention as ONEDPL_GPU: must be set explicitly.
#   USM     -> sycl::malloc_shared (host-addressable, auto-migrated)
#   NO_USM  -> persistent sycl::buffer over the host vector
# See docs/notes/onedpl_gpu_fix_plan.md.
set(PSTL_BENCH_ONEDPL_GPU_MEMORY "" CACHE STRING
        "ONEDPL_GPU memory model: USM | NO_USM (must be set explicitly)")
set_property(CACHE PSTL_BENCH_ONEDPL_GPU_MEMORY PROPERTY STRINGS USM NO_USM)

if (PSTL_BENCH_ONEDPL_GPU_MEMORY STREQUAL "USM")
    add_compile_definitions(PSTL_BENCH_ONEDPL_GPU_USM)
    message(STATUS "ACPP_ONEDPL: memory model = USM (sycl::malloc_shared)")
elseif (PSTL_BENCH_ONEDPL_GPU_MEMORY STREQUAL "NO_USM")
    add_compile_definitions(PSTL_BENCH_ONEDPL_GPU_NO_USM)
    message(STATUS "ACPP_ONEDPL: memory model = NO_USM (persistent sycl::buffer)")
else ()
    message(FATAL_ERROR
            "PSTL_BENCH_ONEDPL_GPU_MEMORY must be set to USM or NO_USM "
            "(got: '${PSTL_BENCH_ONEDPL_GPU_MEMORY}'). "
            "Pass -DPSTL_BENCH_ONEDPL_GPU_MEMORY=USM or =NO_USM at configure time.")
endif ()

# acpp targets. Default `generic` JITs to whichever device
# ACPP_VISIBILITY_MASK enables at runtime.
set(PSTL_BENCH_ACPP_TARGETS "generic" CACHE STRING
        "Value for --acpp-targets= (default: generic)")
set(_acpp_flags "--acpp-targets=${PSTL_BENCH_ACPP_TARGETS}")
message(STATUS "ACPP_ONEDPL: compile/link flags = ${_acpp_flags}")

# Inject --acpp-targets into CMAKE_CXX_FLAGS (CXX-only) so that:
#   - find_package(oneDPL)'s SYCL probes (check_cxx_compiler_flag(-fsycl)
#     and CHECK_INCLUDE_FILE_CXX("sycl/sycl.hpp")) compile through acpp
#     correctly.
#   - C-language probes (e.g. find_package(Threads) inside
#     CPMAddPackage(benchmark)) are unaffected because they use
#     CMAKE_C_FLAGS / CMAKE_EXE_LINKER_FLAGS, neither of which we
#     pollute with --acpp-* flags.
# Linker side is handled per-target via cmake_language(DEFER) below.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_acpp_flags}")

# Same conda-only / no-system-paths policy as the ACPP backend.
# Side effect we rely on: Google Benchmark's `if (BENCHMARK_ENABLE_LIBPFM)
# find_package(PFM)` then misses the system libpfm; src/CMakeLists.txt
# at line 32 (`if (PFM_FOUND)`) silently skips libpfm linkage so
# perf_counters.cc compiles to its no-op variant. (libpfm is not on
# conda-forge, so it isn't available in any env on the stack.)
list(APPEND CMAKE_IGNORE_PATH
        /usr/include /usr/local/include
        /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64)

# Force libbenchmark.a to be PIC. acpp's bundled clang-20 emits non-PIC
# relocations in benchmark.cc.o (R_X86_64_PC32 against libstdc++ symbols
# such as std::__cxx11::basic_string::~basic_string()), and the conda
# toolchain's default PIE link otherwise refuses with "relocation
# R_X86_64_PC32 … can not be used when making a PIE object". The other
# backends (icpx / system gcc / nvc++) emit those dtors with default
# visibility from inline headers, so this only affects acpp.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find oneDPL. With acpp on PATH and --acpp-targets in CMAKE_CXX_FLAGS,
# oneDPL's SYCL probes (check_cxx_compiler_flag(-fsycl) → false for acpp;
# CHECK_INCLUDE_FILE_CXX("sycl/sycl.hpp") → succeeds against AdaptiveCpp
# headers) resolve correctly: oneDPL leaves ONEDPL_USE_DPCPP_BACKEND at
# its default and does NOT inject -fsycl (since the compiler doesn't
# advertise support for it).
find_package(oneDPL REQUIRED)
if (NOT TARGET oneDPL)
    message(FATAL_ERROR "oneDPL target not found")
else ()
    message(STATUS "oneDPL target found")
    list(APPEND LINK_LIBRARIES oneDPL)
endif ()

# Defer link-side --acpp-* onto the project target so:
#   - We don't pollute CMAKE_EXE_LINKER_FLAGS, which would break
#     find_package(Threads) C-language link probes (clang as C linker
#     doesn't understand --acpp-targets).
#   - acpp's driver still gets the flag at link time when wiring up
#     the SYCL runtime libs for the executable.
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_link_options ${PROJECT_NAME} PRIVATE ${_acpp_flags})

# Override Google Benchmark's hidden visibility on the benchmark target
# so libstdc++ inline symbols (std::string dtor in particular) aren't
# emitted as undefined hidden refs in libbenchmark.a — same problem
# the ACPP backend hit, same fix. PIC + default visibility together is
# what makes the link succeed.
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_compile_options benchmark PRIVATE
                "-fvisibility=default" "-fno-visibility-inlines-hidden")

# Google Benchmark probes std::regex with a hardcoded -std=c++11
# try_run, which acpp 25.10 hard-rejects. Pre-set HAVE_STD_REGEX=1 so
# cxx_feature_check short-circuits the probe.
set(HAVE_STD_REGEX 1 CACHE INTERNAL "Bypass Google Benchmark regex probe — acpp rejects -std=c++11")

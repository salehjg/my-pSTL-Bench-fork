# AdaptiveCpp (acpp) backend.
#
# One backend covers both CPU and GPU runs of the same binary:
# --acpp-targets=generic produces device-agnostic IR that the runtime
# JITs to whichever device family is enabled by ACPP_VISIBILITY_MASK
# (cuda / ze / omp). The CPU vs GPU split here exists only to bake
# the right device_type label into the JSON context — it does NOT
# affect compilation. Pattern mirrors how ONEDPL_GPU.cmake forces an
# explicit memory-model choice.
#
# CAUTION: because PSTL_BENCH_USE_GPU is baked at compile time, an
# 'acpp-gpu'-tagged binary run with ACPP_VISIBILITY_MASK=omp will
# label device_type=gpu in the JSON despite executing on CPU. The
# recipe always pairs an acpp-gpu binary with a cuda/ze mask and an
# acpp-cpu binary with an omp mask.

if (NOT CMAKE_CXX_COMPILER MATCHES "acpp$")
    message(WARNING "To use the ACPP backend, set CMAKE_CXX_COMPILER=acpp. Got: ${CMAKE_CXX_COMPILER}")
endif ()

add_compile_definitions(PSTL_BENCH_USE_PSTL)
add_compile_definitions(PSTL_BENCH_BACKEND="ACPP")

# CPU vs GPU device-type label. Required, no default — same convention
# as PSTL_BENCH_ONEDPL_GPU_MEMORY in ONEDPL_GPU.cmake.
set(PSTL_BENCH_ACPP_DEVICE "" CACHE STRING
        "ACPP device tag for JSON context: CPU | GPU (must be set explicitly)")
set_property(CACHE PSTL_BENCH_ACPP_DEVICE PROPERTY STRINGS CPU GPU)

if (PSTL_BENCH_ACPP_DEVICE STREQUAL "GPU")
    add_compile_definitions(PSTL_BENCH_USE_GPU)
    message(STATUS "ACPP: device tag = GPU (PSTL_BENCH_USE_GPU defined)")
elseif (PSTL_BENCH_ACPP_DEVICE STREQUAL "CPU")
    message(STATUS "ACPP: device tag = CPU")
else ()
    message(FATAL_ERROR
            "PSTL_BENCH_ACPP_DEVICE must be set to CPU or GPU "
            "(got: '${PSTL_BENCH_ACPP_DEVICE}'). "
            "Pass -DPSTL_BENCH_ACPP_DEVICE=CPU or =GPU at configure time.")
endif ()

# Compile/link flags. Defaults work on any machine with acpp 25.10+
# on PATH — no hardcoded paths. Override the cache vars below for a
# non-stdpar build or a non-generic target.
set(PSTL_BENCH_ACPP_TARGETS "generic" CACHE STRING
        "Value for --acpp-targets= (default: generic; required for stdpar)")
option(PSTL_BENCH_ACPP_USE_STDPAR
        "Pass --acpp-stdpar (required for std::execution::par offload)" ON)

set(_acpp_flags "--acpp-targets=${PSTL_BENCH_ACPP_TARGETS}")
if (PSTL_BENCH_ACPP_USE_STDPAR)
    list(APPEND _acpp_flags "--acpp-stdpar")
endif ()
message(STATUS "ACPP: compile/link flags = ${_acpp_flags}")

# Don't let any system path leak into the build. Everything must come
# from the conda env stack staged by setvars.all.sh. CMAKE_IGNORE_PATH
# tells find_path/find_library/find_package to skip these dirs even
# when CMake's default-path search would normally include them.
#
# We deliberately rely on a side effect: Google Benchmark (added later
# via CPMAddPackage in the top-level CMakeLists) does
# `if (BENCHMARK_ENABLE_LIBPFM) find_package(PFM)`, then in
# src/CMakeLists.txt:32 only enables libpfm support `if (PFM_FOUND)`.
# Without /usr on the search path, find_package(PFM) misses the
# system libpfm.so, PFM_FOUND stays false, GB skips the libpfm
# linkage and the -DHAVE_LIBPFM define, and perf_counters.cc compiles
# to its no-op variant — so the build needs neither libpfm headers
# nor the system .so. (libpfm is not available on conda-forge, so we
# can't simply add it to acpp_buildenv via create.env.py.)
#
# The conda envs already provide everything else we need: TBB from
# oneapi, libnuma + libstdc++ + libgcc_s from acpp_buildenv, etc. All
# are reachable via CMAKE_PREFIX_PATH (set by setvars.all.sh) which
# CMAKE_IGNORE_PATH does NOT block.
list(APPEND CMAKE_IGNORE_PATH
        /usr/include /usr/local/include
        /usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64)

# Force libbenchmark.a to be PIC. Without this, acpp's bundled clang
# emits non-PIC relocations in benchmark.cc.o (R_X86_64_PC32 against
# libstdc++ symbols like std::__cxx11::basic_string::~basic_string()),
# and linking the PIE pSTL-Bench exe fails with "relocation R_X86_64_PC32
# … can not be used when making a PIE object". Enabling
# POSITION_INDEPENDENT_CODE at directory scope here propagates -fPIC
# into all targets created in this directory and subdirectories,
# including Google Benchmark's library targets added later via
# CPMAddPackage. The pSTL-Bench exe is PIE either way; this just
# makes the static lib it links match.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Override Google Benchmark's hidden visibility on the benchmark target.
# GB's CMakeLists hardcodes -fvisibility=hidden -fvisibility-inlines-hidden
# in its compile flags. acpp's bundled clang-20 then emits the inlined
# libstdc++ std::string destructor inside benchmark.cc.o as an
# *undefined hidden* symbol. Even with -fPIC, an undefined hidden ref
# can't be resolved against the default-visible symbol that libstdc++.so
# exports — the linker rejects it. Other backends (icpx / system gcc /
# nvc++) emit the same dtor with default visibility from inline headers,
# which is why this only bites the acpp build.
#
# Appending -fvisibility=default and -fno-visibility-inlines-hidden to
# the benchmark target via a deferred call (after CPMAddPackage has
# created the target) overrides GB's hidden-visibility flags — last
# -fvisibility= on the command line wins. PIC + default visibility
# together is what makes the link succeed.
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_compile_options benchmark PRIVATE
                "-fvisibility=default" "-fno-visibility-inlines-hidden")

# AdaptiveCpp's stdpar emits a host-fallback path
# (par_unseq_host_fallback_policy) which routes through libstdc++'s
# parallel STL — TBB-backed. Even though we offload to GPU, the binary
# still needs TBB at link time for the fallback. Same find pattern as
# cmake/backends/TBB.cmake uses for the dedicated TBB backend; TBB
# comes from oneapi (on CMAKE_PREFIX_PATH via setvars.all.sh).
find_package(TBB REQUIRED)
if (NOT TARGET TBB::tbb)
    message(FATAL_ERROR "ACPP backend needs TBB (for stdpar host-fallback). TBB::tbb not found.")
endif ()
message(STATUS "ACPP: linking TBB::tbb (for libstdc++ stdpar host-fallback)")
list(APPEND LINK_LIBRARIES TBB::tbb)

# Defer target-level options until after the top-level directory
# finishes processing (i.e. after CPMAddPackage's probes have all run).
# Reasons we cannot push --acpp-* into CMAKE_CXX_FLAGS / CMAKE_EXE_LINKER_FLAGS:
#   - Google Benchmark's CXXFeatureCheck.cmake does try_compile/try_run
#     probes, including C-language probes for find_package(Threads), which
#     inherit CMAKE_EXE_LINKER_FLAGS but use a plain `clang` driver that
#     does not understand --acpp-* flags.
#   - GB also probes std::regex with a hardcoded -std=c++11, which acpp
#     25.10 hard-rejects.
# By the time the deferred call fires, ${PROJECT_NAME} exists and we
# can attach acpp's flags only to it.
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_compile_options ${PROJECT_NAME} PRIVATE ${_acpp_flags})
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_link_options    ${PROJECT_NAME} PRIVATE ${_acpp_flags})

# Also apply --acpp-stdpar to libbenchmark.a. acpp's stdpar plugin
# replaces global new/malloc with USM-aware allocators IN TUs IT
# COMPILES. Without this, libbenchmark.a uses libstdc++'s plain
# allocator while pSTL-Bench's TUs use acpp's USM allocator — and any
# memory that crosses the boundary (e.g., a std::string built inside
# pSTL-Bench code, freed inside GB's reporter, or vice versa) hits an
# allocator mismatch and SEGV's after the bench wrapper returns. GB
# doesn't use std::execution::par anywhere internally, so adding
# --acpp-stdpar to its compile is safe — it only enables the
# allocator-replacement, no accidental GPU offload.
cmake_language(DEFER DIRECTORY "${PROJECT_SOURCE_DIR}"
        CALL target_compile_options benchmark PRIVATE ${_acpp_flags})

# Google Benchmark probes std::regex with a hardcoded -std=c++11
# try_run, which acpp 25.10 hard-rejects ("acpp error: fatal:
# Insufficient c++ standard 'c++11'"). Pre-set HAVE_STD_REGEX=1 so
# cxx_feature_check short-circuits the probe (its DEFINED HAVE_${VAR}
# early-return); the actual std::regex usage at c++20 is fine.
set(HAVE_STD_REGEX 1 CACHE INTERNAL "Bypass Google Benchmark regex probe — acpp rejects -std=c++11")

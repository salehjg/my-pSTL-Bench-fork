#pragma once

#include "pstl/utils/benchmark_naming.h"

// Per-backend benchmark_wrapper. CPU backends fall through to the shared
// host-prep utils. GPU backends pick the matching utils header.
// See docs/notes/cpu_time_vs_real_time.md.
#if defined(PSTL_BENCH_BACKEND_IS_ONEDPL_GPU)
#include "partial_sort_utils_onedpl_gpu.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_ONEDPL)
#include "partial_sort_utils_acpp_onedpl.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_STDPAR)
#include "partial_sort_utils_acpp_stdpar.h"
#elif defined(PSTL_BENCH_BACKEND_IS_NVHPC_CUDA)
#include "partial_sort_utils_nvhpc_cuda.h"
#else
#include "partial_sort_utils.h"
#endif

#include "partial_sort_std.h"

#ifdef PSTL_BENCH_USE_GNU
#include "partial_sort_gnu.h"
#endif

#ifdef PSTL_BENCH_USE_HPX
#include "partial_sort_hpx.h"
#endif

#ifdef PSTL_BENCH_USE_ONEDPL
#include "partial_sort_onedpl.h"
#endif

//region partial_sort_std
template<class Policy>
static void partial_sort_std_wrapper(benchmark::State & state)
{
	benchmark_partial_sort::benchmark_wrapper<Policy>(state, benchmark_partial_sort::partial_sort_std);
}

#ifndef PSTL_BENCH_ONEDPL_GPU_NO_USM
#define PARTIAL_SORT_SEQ_WRAPPER                                                    \
	BENCHMARK_TEMPLATE1(partial_sort_std_wrapper, std::execution::sequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME_WITH_BACKEND("SEQ", "std::partial_sort"))  \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define PARTIAL_SORT_SEQ_WRAPPER
#endif

#ifdef PSTL_BENCH_USE_PSTL
#define PARTIAL_SORT_STD_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(partial_sort_std_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("std::partial_sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define PARTIAL_SORT_STD_WRAPPER
#endif
//endregion partial_sort_std

//region partial_sort_gnu
#ifdef PSTL_BENCH_USE_GNU
template<class Policy>
static void partial_sort_gnu_wrapper(benchmark::State & state)
{
	benchmark_partial_sort::benchmark_wrapper<Policy>(state, benchmark_partial_sort::partial_sort_gnu);
}

#define PARTIAL_SORT_GNU_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(partial_sort_gnu_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("gnu::partial_sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define PARTIAL_SORT_GNU_WRAPPER
#endif
//endregion partial_sort_gnu

//region partial_sort_hpx
#ifdef PSTL_BENCH_USE_HPX
template<class Policy>
static void partial_sort_hpx_wrapper(benchmark::State & state)
{
	benchmark_partial_sort::benchmark_wrapper<Policy>(state, benchmark_partial_sort::partial_sort_hpx);
}

#define PARTIAL_SORT_HPX_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(partial_sort_hpx_wrapper, hpx::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("hpx::partial_sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define PARTIAL_SORT_HPX_WRAPPER
#endif
//endregion partial_sort_hpx

//region partial_sort_onedpl
#ifdef PSTL_BENCH_USE_ONEDPL
template<class Policy>
static void partial_sort_onedpl_wrapper(benchmark::State & state)
{
	benchmark_partial_sort::benchmark_wrapper<Policy>(state, benchmark_partial_sort::partial_sort_onedpl);
}

/*
the std policy is just a placeholder, it will use oneapi::dpl::execution::dpcpp_default when executing the algorithm. 
Check the algorithm implementation.
*/
#define PARTIAL_SORT_ONEDPL_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(partial_sort_onedpl_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("onedpl::partial_sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define PARTIAL_SORT_ONEDPL_WRAPPER
#endif
//endregion partial_sort_onedpl

#define PARTIAL_SORT_GROUP   \
	PARTIAL_SORT_SEQ_WRAPPER \
	PARTIAL_SORT_STD_WRAPPER \
	PARTIAL_SORT_GNU_WRAPPER \
	PARTIAL_SORT_HPX_WRAPPER \
	PARTIAL_SORT_ONEDPL_WRAPPER

PARTIAL_SORT_GROUP
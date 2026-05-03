#pragma once

#include "pstl/utils/benchmark_naming.h"

// Per-backend benchmark_wrapper. CPU backends fall through to the shared
// host-prep utils. GPU backends pick the matching utils header.
// See docs/notes/cpu_time_vs_real_time.md.
#if defined(PSTL_BENCH_BACKEND_IS_ONEDPL_GPU)
#include "sort_utils_onedpl_gpu.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_ONEDPL)
#include "sort_utils_acpp_onedpl.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_STDPAR)
#include "sort_utils_acpp_stdpar.h"
#elif defined(PSTL_BENCH_BACKEND_IS_NVHPC_CUDA)
#include "sort_utils_nvhpc_cuda.h"
#else
#include "sort_utils.h"
#endif

#include "sort_std.h"

#ifdef PSTL_BENCH_USE_GNU
#include "sort_gnu.h"
#endif //USE_GNU

#ifdef PSTL_BENCH_USE_HPX
#include "sort_hpx.h"
#endif //USE_HPX

#ifdef PSTL_BENCH_USE_ONEDPL
#include "sort_onedpl.h"
#include <oneapi/dpl/execution>
#endif

//region sort_std
template<class Policy>
static void sort_std_wrapper(benchmark::State & state)
{
	benchmark_sort::benchmark_wrapper<Policy>(state, benchmark_sort::sort_std);
}

#ifndef PSTL_BENCH_ONEDPL_GPU_NO_USM
#define SORT_SEQ_WRAPPER                                                    \
	BENCHMARK_TEMPLATE1(sort_std_wrapper, std::execution::sequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME_WITH_BACKEND("SEQ", "std::sort"))  \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SORT_SEQ_WRAPPER
#endif

#ifdef PSTL_BENCH_USE_PSTL
#define SORT_STD_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(sort_std_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("std::sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SORT_STD_WRAPPER
#endif
//endregion sort_std

//region sort_gnu
#ifdef PSTL_BENCH_USE_GNU
template<class Policy>
static void sort_gnu_wrapper(benchmark::State & state)
{
	benchmark_sort::benchmark_wrapper<Policy>(state, benchmark_sort::sort_gnu);
}

#define SORT_GNU_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(sort_gnu_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("gnu::sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SORT_GNU_WRAPPER
#endif
//endregion sort_gnu

//region sort_hpx
#ifdef PSTL_BENCH_USE_HPX
template<class Policy>
static void sort_hpx_wrapper(benchmark::State & state)
{
	benchmark_sort::benchmark_wrapper<Policy>(state, benchmark_sort::sort_hpx);
}

#define SORT_HPX_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(sort_hpx_wrapper, hpx::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("hpx::sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SORT_HPX_WRAPPER
#endif
//endregion sort_hpx

//region sort_onedpl
#ifdef PSTL_BENCH_USE_ONEDPL
template<class Policy>
static void sort_onedpl_wrapper(benchmark::State & state)
{
	benchmark_sort::benchmark_wrapper<Policy>(state, benchmark_sort::sort_onedpl);
}

/*
the std policy is just a placeholder, it will use oneapi::dpl::execution::dpcpp_default when executing the algorithm. 
Check the algorithm implementation.
*/
#define SORT_ONEDPL_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(sort_onedpl_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("onedpl::sort"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SORT_ONEDPL_WRAPPER
#endif
//endregion sort_onedpl

#define SORT_GROUP   \
	SORT_SEQ_WRAPPER \
	SORT_STD_WRAPPER \
	SORT_GNU_WRAPPER \
	SORT_HPX_WRAPPER \
	SORT_ONEDPL_WRAPPER

SORT_GROUP
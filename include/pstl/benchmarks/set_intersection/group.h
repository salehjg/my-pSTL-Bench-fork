#pragma once

#include "pstl/utils/benchmark_naming.h"

// Per-backend benchmark_wrapper. CPU backends fall through to the shared
// host-prep utils. GPU backends pick the matching utils header so the
// per-iter output reset (std::fill) runs on the device.
// See docs/notes/cpu_time_vs_real_time.md.
#if defined(PSTL_BENCH_BACKEND_IS_ONEDPL_GPU)
#include "set_intersection_utils_onedpl_gpu.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_ONEDPL)
#include "set_intersection_utils_acpp_onedpl.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_STDPAR)
#include "set_intersection_utils_acpp_stdpar.h"
#elif defined(PSTL_BENCH_BACKEND_IS_NVHPC_CUDA)
#include "set_intersection_utils_nvhpc_cuda.h"
#else
#include "set_intersection_utils.h"
#endif

#include "set_intersection_std.h"

#ifdef PSTL_BENCH_USE_GNU
#include "set_intersection_gnu.h"
#endif

#ifdef PSTL_BENCH_USE_HPX
#include "set_intersection_hpx.h"
#include <hpx/executors/parallel_executor.hpp>
#endif

#ifdef PSTL_BENCH_USE_ONEDPL
#include "set_intersection_onedpl.h"
#endif

//region set_intersection_std
template<class Policy>
static void set_intersection_std_wrapper(benchmark::State & state)
{
	benchmark_set_intersection::benchmark_wrapper<Policy>(state, benchmark_set_intersection::set_intersection_std);
}

#ifndef PSTL_BENCH_ONEDPL_GPU_NO_USM
#define SET_INTERSECTION_SEQ_WRAPPER                                                    \
	BENCHMARK_TEMPLATE1(set_intersection_std_wrapper, std::execution::sequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME_WITH_BACKEND("SEQ", "std::set_intersection"))  \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SET_INTERSECTION_SEQ_WRAPPER
#endif

#ifdef PSTL_BENCH_USE_PSTL
#define SET_INTERSECTION_STD_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(set_intersection_std_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("std::set_intersection"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SET_INTERSECTION_STD_WRAPPER
#endif
//endregion set_intersection_std

//region set_intersection_gnu
#ifdef PSTL_BENCH_USE_GNU
template<class Policy>
static void set_intersection_gnu_wrapper(benchmark::State & state)
{
	benchmark_set_intersection::benchmark_wrapper<Policy>(state, benchmark_set_intersection::set_intersection_gnu);
}

#define SET_INTERSECTION_GNU_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(set_intersection_gnu_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("gnu::set_intersection"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SET_INTERSECTION_GNU_WRAPPER
#endif
//endregion set_intersection_gnu

//region set_intersection_hpx
#ifdef PSTL_BENCH_USE_HPX
template<class Policy>
static void set_intersection_hpx_wrapper(benchmark::State & state)
{
	benchmark_set_intersection::benchmark_wrapper<Policy>(state, benchmark_set_intersection::set_intersection_hpx);
}

#define SET_INTERSECTION_HPX_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(set_intersection_hpx_wrapper, hpx::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("hpx::set_intersection"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SET_INTERSECTION_HPX_WRAPPER
#endif
//endregion set_intersection_hpx

//region set_intersection_onedpl
#ifdef PSTL_BENCH_USE_ONEDPL
template<class Policy>
static void set_intersection_onedpl_wrapper(benchmark::State & state)
{
	benchmark_set_intersection::benchmark_wrapper<Policy>(state, benchmark_set_intersection::set_intersection_onedpl);
}

/*
the std policy is just a placeholder, it will use oneapi::dpl::execution::dpcpp_default when executing the algorithm. 
Check the algorithm implementation.
*/
#define SET_INTERSECTION_ONEDPL_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(set_intersection_onedpl_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("onedpl::set_intersection"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define SET_INTERSECTION_ONEDPL_WRAPPER
#endif
//endregion set_intersection_onedpl

#define SET_INTERSECTION_GROUP   \
	SET_INTERSECTION_SEQ_WRAPPER \
	SET_INTERSECTION_STD_WRAPPER \
	SET_INTERSECTION_GNU_WRAPPER \
	SET_INTERSECTION_HPX_WRAPPER \
	SET_INTERSECTION_ONEDPL_WRAPPER

SET_INTERSECTION_GROUP
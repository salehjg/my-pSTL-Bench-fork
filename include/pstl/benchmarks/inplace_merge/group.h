#pragma once

#include "pstl/utils/benchmark_naming.h"

// Per-backend benchmark_wrapper. CPU backends (TBB, GNU, HPX, NVHPC_OMP,
// ONEDPL_CPU, ACPP-CPU) fall through to the shared host-prep utils. GPU
// backends pick the matching utils header so the per-iter prep sorts run
// on the device. See docs/notes/cpu_time_vs_real_time.md.
#if defined(PSTL_BENCH_BACKEND_IS_ONEDPL_GPU)
#include "inplace_merge_utils_onedpl_gpu.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_ONEDPL)
#include "inplace_merge_utils_acpp_onedpl.h"
#elif defined(PSTL_BENCH_BACKEND_IS_ACPP_STDPAR)
#include "inplace_merge_utils_acpp_stdpar.h"
#elif defined(PSTL_BENCH_BACKEND_IS_NVHPC_CUDA)
#include "inplace_merge_utils_nvhpc_cuda.h"
#else
#include "inplace_merge_utils.h"
#endif

#include "inplace_merge_std.h"

#ifdef PSTL_BENCH_USE_HPX
#include "inplace_merge_hpx.h"
#endif

#ifdef PSTL_BENCH_USE_ONEDPL
#include "inplace_merge_onedpl.h"
#endif

//region inplace_merge_std
template<class Policy>
static void inplace_merge_std_wrapper(benchmark::State & state)
{
	benchmark_inplace_merge::benchmark_wrapper<Policy>(state, benchmark_inplace_merge::inplace_merge_std);
}

#ifndef PSTL_BENCH_ONEDPL_GPU_NO_USM
#define INPLACE_MERGE_SEQ_WRAPPER                                                    \
	BENCHMARK_TEMPLATE1(inplace_merge_std_wrapper, std::execution::sequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME_WITH_BACKEND("SEQ", "std::inplace_merge"))  \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define INPLACE_MERGE_SEQ_WRAPPER
#endif

#ifdef PSTL_BENCH_USE_PSTL
#define INPLACE_MERGE_STD_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(inplace_merge_std_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("std::inplace_merge"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define INPLACE_MERGE_STD_WRAPPER
#endif
//endregion inplace_merge_std

//region inplace_merge_hpx
#ifdef PSTL_BENCH_USE_HPX
template<class Policy>
static void inplace_merge_hpx_wrapper(benchmark::State & state)
{
	benchmark_inplace_merge::benchmark_wrapper<Policy>(state, benchmark_inplace_merge::inplace_merge_hpx);
}

#define INPLACE_MERGE_HPX_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(inplace_merge_hpx_wrapper, hpx::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("hpx::inplace_merge"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define INPLACE_MERGE_HPX_WRAPPER
#endif
//endregion inplace_merge_hpx

//region inplace_merge_onedpl
#ifdef PSTL_BENCH_USE_ONEDPL
template<class Policy>
static void inplace_merge_onedpl_wrapper(benchmark::State & state)
{
	benchmark_inplace_merge::benchmark_wrapper<Policy>(state, benchmark_inplace_merge::inplace_merge_onedpl);
}

/*
the std policy is just a placeholder, it will use oneapi::dpl::execution::dpcpp_default when executing the algorithm. 
Check the algorithm implementation.
*/
#define INPLACE_MERGE_ONEDPL_WRAPPER                                                               \
	BENCHMARK_TEMPLATE1(inplace_merge_onedpl_wrapper, std::execution::parallel_unsequenced_policy) \
	    ->Name(PSTL_BENCH_BENCHMARK_NAME("onedpl::inplace_merge"))                                 \
	    ->PSTL_BENCH_BENCHMARK_PARAMETERS
#else
#define INPLACE_MERGE_ONEDPL_WRAPPER
#endif
//endregion inplace_merge_onedpl

#define INPLACE_MERGE_GROUP   \
	INPLACE_MERGE_SEQ_WRAPPER \
	INPLACE_MERGE_STD_WRAPPER \
	INPLACE_MERGE_HPX_WRAPPER \
	INPLACE_MERGE_ONEDPL_WRAPPER

INPLACE_MERGE_GROUP
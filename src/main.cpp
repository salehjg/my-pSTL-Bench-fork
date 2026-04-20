#ifdef PSTL_BENCH_USE_HPX
#include <hpx/hpx_main.hpp>
#endif

#include <thread>

#include <benchmark/benchmark.h>

#if defined(PSTL_BENCH_USE_TBB) or defined(PSTL_BENCH_USE_HPX)
#include "pstl/utils/thread_control.h"
#endif

#ifdef PSTL_BENCH_USE_LIKWID
#include <likwid-marker.h>
#endif

#include "pstl/utils/profiler_markers.h"
#include "pstl/utils/benchmark_naming.h"

#include "pstl/benchmarks/pstl-benchmarks.h"

// Run the benchmark
int main(int argc, char ** argv)
{
#ifdef PSTL_BENCH_USE_TBB
	auto tbbThreadControl = init_tbb_thread_control();
#endif

	pstl::profiler_init();

	benchmark::AddCustomContext("std::thread::hardware_concurrency()",
	                            std::to_string(std::thread::hardware_concurrency()));

#ifdef PSTL_BENCH_COMPILER
	benchmark::AddCustomContext("compiler", XSTRINGIFY(PSTL_BENCH_COMPILER));
#endif

#ifdef PSTL_BENCH_BACKEND
	benchmark::AddCustomContext("backend", XSTRINGIFY(PSTL_BENCH_BACKEND));
#endif

#if defined(PSTL_BENCH_USE_NVTX)
	benchmark::AddCustomContext("profiler_markers", "NVTX");
#elif defined(PSTL_BENCH_USE_ITT)
	benchmark::AddCustomContext("profiler_markers", "ITT");
#elif defined(PSTL_BENCH_USE_ROCTX)
	benchmark::AddCustomContext("profiler_markers", "ROCTX");
#else
	benchmark::AddCustomContext("profiler_markers", "none");
#endif

#if defined(PSTL_BENCH_ONEDPL_GPU_USM)
	benchmark::AddCustomContext("gpu_memory", "USM_shared");
#elif defined(PSTL_BENCH_USE_GPU)
	benchmark::AddCustomContext("gpu_memory", "host_iterators");
#endif

#if defined(PSTL_BENCH_USE_TBB)
	benchmark::AddCustomContext("tbb #threads", std::to_string(tbb::global_control::active_value(
	                                                tbb::global_control::max_allowed_parallelism)));
#elif defined(PSTL_BENCH_USE_GNU_PSTL)
	benchmark::AddCustomContext("omp #threads", std::to_string(omp_get_max_threads()));
#elif defined(PSTL_BENCH_USE_HPX)
	benchmark::AddCustomContext("hpx #threads", std::to_string(hpx::get_num_worker_threads()));
#endif

#ifdef PSTL_BENCH_USE_PAPI
	benchmark::AddCustomContext("PAPI", "enabled");
#endif

#ifdef PSTL_BENCH_USE_LIKWID
	benchmark::AddCustomContext("LIKWID", "enabled");
	LIKWID_MARKER_INIT;
#endif

	benchmark::Initialize(&argc, argv);
	if (benchmark::ReportUnrecognizedArguments(argc, argv)) { return 1; }
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();

#ifdef PSTL_BENCH_USE_LIKWID
	LIKWID_MARKER_CLOSE;
#endif

	return EXIT_SUCCESS;
}

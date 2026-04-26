#ifdef PSTL_BENCH_USE_HPX
#include <hpx/hpx_main.hpp>
#endif

#include <cstdlib>
#include <fstream>
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

#ifdef PSTL_BENCH_USE_GPU
	benchmark::AddCustomContext("device_type", "gpu");
#else
	benchmark::AddCustomContext("device_type", "cpu");
#endif

#if defined(PSTL_BENCH_ONEDPL_GPU_USM)
	benchmark::AddCustomContext("gpu_memory", "USM_shared");
#elif defined(PSTL_BENCH_ONEDPL_GPU_NO_USM)
	benchmark::AddCustomContext("gpu_memory", "persistent_buffer");
#elif defined(PSTL_BENCH_USE_GPU)
	// NVHPC_CUDA, NVHPC_OMP target offload — no oneDPL memory model
	benchmark::AddCustomContext("gpu_memory", "n/a");
#else
	benchmark::AddCustomContext("gpu_memory", "none");
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
	benchmark::AddCustomContext("opt_papi", "ON");
#else
	benchmark::AddCustomContext("opt_papi", "OFF");
#endif

#ifdef PSTL_BENCH_USE_LIKWID
	benchmark::AddCustomContext("LIKWID", "enabled");
	benchmark::AddCustomContext("opt_likwid", "ON");
	LIKWID_MARKER_INIT;
#else
	benchmark::AddCustomContext("opt_likwid", "OFF");
#endif

#ifdef PSTL_BENCH_USE_NVTX
	benchmark::AddCustomContext("opt_nvtx", "ON");
#else
	benchmark::AddCustomContext("opt_nvtx", "OFF");
#endif

#ifdef PSTL_BENCH_USE_ITT
	benchmark::AddCustomContext("opt_itt", "ON");
#else
	benchmark::AddCustomContext("opt_itt", "OFF");
#endif

#ifdef PSTL_BENCH_USE_ROCTX
	benchmark::AddCustomContext("opt_roctx", "ON");
#else
	benchmark::AddCustomContext("opt_roctx", "OFF");
#endif

#ifdef PSTL_BENCH_USE_PAR_ALLOC
	benchmark::AddCustomContext("opt_par_alloc", "ON");
#else
	benchmark::AddCustomContext("opt_par_alloc", "OFF");
#endif

	benchmark::Initialize(&argc, argv);
	if (benchmark::ReportUnrecognizedArguments(argc, argv)) { return 1; }
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();

#ifdef PSTL_BENCH_USE_LIKWID
	LIKWID_MARKER_CLOSE;
#endif

	// Write status file to signal clean completion (used by recipe runners
	// to detect killed processes and resume from where they left off).
	const char * status_path = std::getenv("PSTL_BENCH_STATUS_FILE");
	if (status_path)
	{
		std::ofstream(status_path) << "OK" << std::endl;
	}

	return EXIT_SUCCESS;
}

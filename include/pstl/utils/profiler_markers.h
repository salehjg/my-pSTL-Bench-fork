#pragma once

// Vendor-specific GPU profiler marker API.
// Exactly one of PSTL_BENCH_USE_NVTX, PSTL_BENCH_USE_ITT, PSTL_BENCH_USE_ROCTX
// may be defined at compile time.  When none is defined every function below is
// an empty inline that the compiler will eliminate entirely.
//
// Each call to profiler_range_push() appends an incrementing iteration counter
// per unique range name (e.g. ".../for_each/double/1048576/iter_3").  Combined
// with a known measurement iteration count (--benchmark_min_time=Nx), the last
// N iterations per benchmark are measurement; everything before is warmup.

#include <string>
#include <unordered_map>

#ifdef PSTL_BENCH_USE_NVTX
#include <nvtx3/nvToolsExt.h>
#endif

#ifdef PSTL_BENCH_USE_ITT
#include <ittnotify.h>
#endif

#ifdef PSTL_BENCH_USE_ROCTX
#include <roctx.h>
#endif

namespace pstl
{
	/// Call once from main() before any benchmarks run.
	/// ITT needs a domain created up front; NVTX and roctx are no-ops.
	inline void profiler_init()
	{
#ifdef PSTL_BENCH_USE_ITT
		__itt_domain_create("pstl_bench");
#endif
	}

	/// Push a named range onto the profiler stack.
	/// Appends "/iter_N" (N = per-name call count) so that warmup and
	/// measurement iterations can be distinguished in post-processing.
	inline void profiler_range_push(const char * name)
	{
#if defined(PSTL_BENCH_USE_NVTX) || defined(PSTL_BENCH_USE_ITT) || defined(PSTL_BENCH_USE_ROCTX)
		static std::unordered_map<std::string, int> counters;
		int & count      = counters[name];
		std::string tagged = std::string(name) + "/iter_" + std::to_string(count);
		count++;

#if defined(PSTL_BENCH_USE_NVTX)
		nvtxRangePushA(tagged.c_str());
#elif defined(PSTL_BENCH_USE_ITT)
		static __itt_domain * domain = __itt_domain_create("pstl_bench");
		__itt_string_handle * handle = __itt_string_handle_create(tagged.c_str());
		__itt_task_begin(domain, __itt_null, __itt_null, handle);
#elif defined(PSTL_BENCH_USE_ROCTX)
		roctxRangePushA(tagged.c_str());
#endif
#endif
		(void)name; // suppress unused-parameter warning when no marker is enabled
	}

	/// Pop the most recent range from the profiler stack.
	inline void profiler_range_pop()
	{
#ifdef PSTL_BENCH_USE_NVTX
		nvtxRangePop();
#elif defined(PSTL_BENCH_USE_ITT)
		static __itt_domain * domain = __itt_domain_create("pstl_bench");
		__itt_task_end(domain);
#elif defined(PSTL_BENCH_USE_ROCTX)
		roctxRangePop();
#endif
	}
} // namespace pstl

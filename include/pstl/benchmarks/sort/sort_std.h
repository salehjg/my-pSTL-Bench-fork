#pragma once

#include <algorithm>
#include <functional>

#if defined(PSTL_BENCH_USE_TBB)
#include <tbb/parallel_sort.h>
#endif //USE_TBB

namespace benchmark_sort
{
	const auto sort_std = [](auto && policy, auto & input) {
#if defined(PSTL_BENCH_USE_TBB)
	// Show compiler message if TBB is used
#warning \
    "Using tbb::parallel_sort since std::sort(parallel_policy) explodes in memory usage. See: https://github.com/llvm/llvm-project/issues/78956#issue-2093630075"
		tbb::parallel_sort(input.begin(), input.end());
#elif defined(__ACPP__)
		// AdaptiveCpp 25.10 stdpar bug — the 3-arg
		// std::sort(par_unseq, first, last) interceptor in
		// hipSYCL/std/stdpar/pstl-impl/algorithm.hpp:1454
		// calls hipsycl::algorithms::sort(queue, first, last) with no
		// Compare argument, but the only overload of that algorithm
		// (algorithms/algorithm.hpp:1226) declares Compare as a template
		// parameter — its `= std::less<>{}` is a default *value*, not a
		// default *type*, so deduction fails. Pass std::less<>{}
		// explicitly to route through the working 4-arg interceptor.
		// See docs/notes/acpp_stdpar_sort_template_bug.md. Drop this
		// branch once AdaptiveCpp ships a fix upstream.
		std::sort(policy, input.begin(), input.end(), std::less<>{});
#else
		std::sort(policy, input.begin(), input.end());
#endif //USE_TBB
	};
} // namespace benchmark_sort

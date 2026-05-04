#pragma once

#include <algorithm>
#include <cstddef>

#ifdef PSTL_BENCH_USE_GPU
#include <tbb/parallel_sort.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#endif

namespace pstl
{
	template<typename It>
	void host_sort(It first, It last)
	{
#ifdef PSTL_BENCH_USE_GPU
		tbb::parallel_sort(first, last);
#else
		std::sort(first, last);
#endif
	}

	template<typename It, typename T>
	void host_fill(It first, It last, const T & value)
	{
#ifdef PSTL_BENCH_USE_GPU
		const auto n = std::distance(first, last);
		tbb::parallel_for(tbb::blocked_range<std::ptrdiff_t>(0, n),
		                   [first, &value](const auto & r) {
			                   std::fill(first + r.begin(), first + r.end(), value);
		                   });
#else
		std::fill(first, last, value);
#endif
	}
} // namespace pstl

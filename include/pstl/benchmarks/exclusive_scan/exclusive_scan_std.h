#pragma once

#include <execution>
#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/elem_t.h"

namespace benchmark_exclusive_scan
{
	const auto exclusive_scan_std = [](auto && policy, const auto & input, auto & output) {
		std::exclusive_scan(policy, input.begin(), input.end(), output.begin(), pstl::elem_t{});
	};
} // namespace benchmark_exclusive_scan

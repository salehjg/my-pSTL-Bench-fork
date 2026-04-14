#pragma once

#include <hpx/numeric.hpp>

#include "pstl/utils/elem_t.h"

namespace benchmark_exclusive_scan
{
	const auto exclusive_scan_hpx = [](auto && policy, const auto & input, auto & output) {
		hpx::exclusive_scan(policy, input.begin(), input.end(), output.begin(), pstl::elem_t{});
	};
} // namespace benchmark_exclusive_scan

#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/elem_t.h"

namespace benchmark_exclusive_scan
{
	const auto exclusive_scan_onedpl = [](auto && policy, const auto & input, auto & output) {
		oneapi::dpl::exclusive_scan(oneapi::dpl::execution::dpcpp_default, input.begin(), input.end(), output.begin(),
		                            pstl::elem_t{});
	};
} // namespace benchmark_exclusive_scan

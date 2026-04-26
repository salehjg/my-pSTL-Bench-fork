#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

#include "pstl/utils/elem_t.h"

namespace benchmark_exclusive_scan
{
	const auto exclusive_scan_onedpl = [](auto && policy, const auto & input, auto & output) {
		oneapi::dpl::exclusive_scan(pstl::onedpl_gpu_policy(), input.begin(), input.end(), output.begin(),
		                            pstl::elem_t{});
	};
} // namespace benchmark_exclusive_scan

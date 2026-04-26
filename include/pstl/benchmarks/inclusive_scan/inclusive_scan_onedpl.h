#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_inclusive_scan
{
	const auto inclusive_onedpl = [](auto && policy, const auto & input, auto & output) {
		oneapi::dpl::inclusive_scan(pstl::onedpl_gpu_policy(), input.begin(), input.end(), output.begin());
	};
} // namespace benchmark_inclusive_scan

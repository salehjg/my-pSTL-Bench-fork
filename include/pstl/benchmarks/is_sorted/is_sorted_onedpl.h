#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_is_sorted
{
	const auto is_sorted_onedpl = [](auto && policy, const auto & input) {
		return oneapi::dpl::is_sorted(pstl::onedpl_gpu_policy(), input.begin(), input.end());
	};
} // namespace benchmark_is_sorted

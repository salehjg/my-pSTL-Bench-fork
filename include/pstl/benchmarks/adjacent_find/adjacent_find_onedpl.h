#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_adjacent_find
{
	const auto adjacent_find_onedpl = [](auto && policy, const auto & input) {
		return oneapi::dpl::adjacent_find(pstl::onedpl_gpu_policy(), input.begin(), input.end());
	};
} // namespace benchmark_adjacent_find

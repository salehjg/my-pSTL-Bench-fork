
#pragma once

#include <vector>

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_find
{
	const auto find_onedpl = [](auto && policy, const auto & input, const auto & find_val) {
		return oneapi::dpl::find(pstl::onedpl_gpu_policy(), input.begin(), input.end(), find_val);
	};
} // namespace benchmark_find

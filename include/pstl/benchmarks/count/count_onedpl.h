#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_count
{
	const auto count_onedpl = [](auto && policy, const auto & input, const auto & value) {
		return oneapi::dpl::count(pstl::onedpl_gpu_policy(), input.begin(), input.end(), value);
	};
} // namespace benchmark_count

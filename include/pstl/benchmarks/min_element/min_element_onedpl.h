#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_min_element
{
	const auto min_element_onedpl = [](auto && policy, const auto & input) {
		return oneapi::dpl::min_element(pstl::onedpl_gpu_policy(), input.begin(), input.end());
	};
} // namespace benchmark_min_element

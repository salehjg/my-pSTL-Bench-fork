#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_max_element
{
	const auto max_element_onedpl = [](auto && policy, const auto & input) {
		return oneapi::dpl::max_element(pstl::onedpl_gpu_policy(), input.begin(), input.end());
	};
} // namespace benchmark_max_element

#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_adjacent_difference
{
	const auto adjacent_difference_onedpl = [](auto & policy, const auto & input, auto & output) {
		oneapi::dpl::adjacent_difference(pstl::onedpl_gpu_policy(), input.begin(), input.end(),
		                                 output.begin());
	};
} // namespace benchmark_adjacent_difference

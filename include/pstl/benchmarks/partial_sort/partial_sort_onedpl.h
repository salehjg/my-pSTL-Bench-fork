#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_partial_sort
{
	const auto partial_sort_onedpl = [](auto && policy, auto begin, auto middle, auto end) {
		return oneapi::dpl::partial_sort(pstl::onedpl_gpu_policy(), begin, middle, end);
	};
} // namespace benchmark_partial_sort

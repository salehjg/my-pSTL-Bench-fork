#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_sort
{
	const auto sort_onedpl = [](auto && policy, auto & input) {
		oneapi::dpl::sort(pstl::onedpl_gpu_policy(), input.begin(), input.end());
	};

} // namespace benchmark_sort
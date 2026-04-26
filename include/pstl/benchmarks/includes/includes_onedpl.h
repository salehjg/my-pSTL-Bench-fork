#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_includes
{
	const auto includes_onedpl = [](auto && policy, const auto & input, const auto & subset) {
		return oneapi::dpl::includes(pstl::onedpl_gpu_policy(), input.begin(), input.end(), subset.begin(),
		                             subset.end());
	};
} // namespace benchmark_includes

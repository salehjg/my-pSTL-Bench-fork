#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_any_of
{

	const auto any_of_onedpl = [](auto && policy, const auto & input, auto && condition) {
		return oneapi::dpl::any_of(pstl::onedpl_gpu_policy(), input.begin(), input.end(), condition);
	};

} // namespace benchmark_any_of

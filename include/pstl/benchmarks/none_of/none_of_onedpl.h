
#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_none_of
{

	const auto none_of_onedpl = [](auto && policy, const auto & input, auto && condition) {
		return oneapi::dpl::none_of(pstl::onedpl_gpu_policy(), input.begin(), input.end(), condition);
	};

} // namespace benchmark_none_of

#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_fill
{
	const auto fill_onedpl = [](auto && policy, auto & input, const auto & value) {
		return oneapi::dpl::fill(pstl::onedpl_gpu_policy(), input.begin(), input.end(), value);
	};
} // namespace benchmark_fill

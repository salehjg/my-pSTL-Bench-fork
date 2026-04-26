#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_generate
{
	const auto generate_onedpl = [](auto && policy, auto & input, auto && generator) {
		return oneapi::dpl::generate(pstl::onedpl_gpu_policy(), input.begin(), input.end(), generator);
	};
} // namespace benchmark_generate

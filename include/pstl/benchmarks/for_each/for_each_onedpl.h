#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_for_each
{
	const auto for_each_onedpl = [](auto && policy, auto & input, auto && f) {
		oneapi::dpl::for_each(pstl::onedpl_gpu_policy(), input.begin(), input.end(), f);
	};

} // namespace benchmark_for_each

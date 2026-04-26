#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_equal
{
	const auto equal_onedpl = [](auto && policy, const auto & input, auto & output) {
		oneapi::dpl::equal(pstl::onedpl_gpu_policy(), input.begin(), input.end(), output.begin());
	};
} // namespace benchmark_equal

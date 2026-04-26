#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_set_difference
{
	const auto set_difference_onedpl = [](auto && policy, auto & data1, auto & data2, auto & output) {
		return oneapi::dpl::set_difference(pstl::onedpl_gpu_policy(), data1.begin(), data1.end(),
		                                   data2.begin(), data2.end(), output.begin());
	};
} // namespace benchmark_set_difference

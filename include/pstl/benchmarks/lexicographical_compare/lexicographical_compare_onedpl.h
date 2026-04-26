#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_lexicographical_compare
{
	const auto lexicographical_compare_onedpl = [](auto && policy, const auto & data_a, const auto & data_b) {
		return oneapi::dpl::lexicographical_compare(pstl::onedpl_gpu_policy(), data_a.begin(), data_a.end(),
		                                            data_b.begin(), data_b.end());
	};
} // namespace benchmark_lexicographical_compare

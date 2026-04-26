#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_partition
{
	const auto partition_onedpl = [](auto && policy, auto & input, auto && condition) {
		return oneapi::dpl::partition(pstl::onedpl_gpu_policy(), input.begin(), input.end(),
		                              std::forward<decltype(condition)>(condition));
	};
} // namespace benchmark_partition

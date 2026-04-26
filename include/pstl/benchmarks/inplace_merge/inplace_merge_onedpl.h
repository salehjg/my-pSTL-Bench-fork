#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_inplace_merge
{
	const auto inplace_merge_onedpl = [](auto && policy, auto begin, auto middle, auto end) {
		return oneapi::dpl::inplace_merge(pstl::onedpl_gpu_policy(), begin, middle, end);
	};
} // namespace benchmark_inplace_merge

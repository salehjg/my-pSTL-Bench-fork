#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_merge
{
	const auto merge_onedpl = [](auto && policy, auto first1, auto last1, auto first2, auto last2, auto d_first) {
		return oneapi::dpl::merge(pstl::onedpl_gpu_policy(), first1, last1, first2, last2, d_first);
	};
} // namespace benchmark_merge

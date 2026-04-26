#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

#include <benchmark/benchmark.h>

namespace benchmark_count_if
{
	const auto count_if_onedpl = [](auto && policy, const auto & input, auto & condition) {
		return oneapi::dpl::count_if(pstl::onedpl_gpu_policy(), input.begin(), input.end(), condition);
	};
} // namespace benchmark_count_if

#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_copy_n
{
	const auto copy_n_onedpl = [](auto && policy, const auto & input, const auto & n, auto & output) {
		oneapi::dpl::copy_n(pstl::onedpl_gpu_policy(), input.begin(), n, output.begin());
	};
} // namespace benchmark_copy_n

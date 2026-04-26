#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_copy_if
{
	const auto copy_if_onedpl = [](auto && policy, const auto & input, auto & output, auto && condition) {
		oneapi::dpl::copy_if(pstl::onedpl_gpu_policy(), input.begin(), input.end(), output.begin(),
		                     condition);
	};
} // namespace benchmark_copy_if

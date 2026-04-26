#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_transform_inclusive_scan
{

	const auto transform_inclusive_scan_onedpl = [](auto && policy, auto & input, auto & output, auto && f) {
		return oneapi::dpl::transform_inclusive_scan(pstl::onedpl_gpu_policy(), input.begin(), input.end(),
		                                             output.begin(), std::plus<>(), f);
	};

} // namespace benchmark_transform_inclusive_scan

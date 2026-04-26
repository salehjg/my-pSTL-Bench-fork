#pragma once

#include <functional>
#include <utility>

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

#include "pstl/utils/elem_t.h"

namespace benchmark_transform_exclusive_scan
{
	const auto transform_exclusive_scan_onedpl = [](auto && policy, auto & input, auto & output, auto && f) {
		return oneapi::dpl::transform_exclusive_scan(pstl::onedpl_gpu_policy(), input.begin(), input.end(),
		                                             output.begin(), pstl::elem_t{}, std::plus<>(), f);
	};
} // namespace benchmark_transform_exclusive_scan

#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_transform
{

	const auto transform_onedpl = [](auto && policy, auto & input, auto & output, auto && f) {
		oneapi::dpl::transform(pstl::onedpl_gpu_policy(), input.begin(), input.end(), output.begin(), f);
	};

} // namespace benchmark_transform

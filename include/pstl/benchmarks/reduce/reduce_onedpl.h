#pragma once

#include "pstl/utils/elem_t.h"

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

#include "pstl/utils/onedpl_gpu_policy.h"

namespace benchmark_reduce
{
	const auto reduce_onedpl = [](auto && policy, const auto & input) {
		return oneapi::dpl::reduce(pstl::onedpl_gpu_policy(), input.begin(), input.end(), pstl::elem_t{});
	};
} // namespace benchmark_reduce

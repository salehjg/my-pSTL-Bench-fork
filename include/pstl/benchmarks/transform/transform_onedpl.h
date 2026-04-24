#pragma once

#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>

namespace benchmark_transform
{

	const auto transform_onedpl = [](auto && policy, auto & input, auto & output, auto && f) {
		oneapi::dpl::transform(oneapi::dpl::execution::dpcpp_default, input.begin(), input.end(), output.begin(), f);
	};

} // namespace benchmark_transform

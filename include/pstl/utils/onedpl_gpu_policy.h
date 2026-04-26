#pragma once

// Process-wide oneDPL device policy bound to pstl::onedpl_gpu_queue().
//
// All *_onedpl.h wrappers submit through this policy instead of
// oneapi::dpl::execution::dpcpp_default, so that policy queue, USM allocator
// queue, and any sycl::buffer accessors all share one SYCL context.

#include <oneapi/dpl/execution>

#include "pstl/utils/sycl_queue.h"

namespace pstl
{
	inline auto & onedpl_gpu_policy()
	{
		static auto policy = oneapi::dpl::execution::make_device_policy(onedpl_gpu_queue());
		return policy;
	}
} // namespace pstl

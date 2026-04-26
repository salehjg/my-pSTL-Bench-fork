#pragma once

// Process-wide sycl::queue used by the ONEDPL_GPU backend.
//
// Both the USM allocator and the oneDPL execution policy bind to this queue
// so that allocations and kernel submissions share the same SYCL context.
// Without that, oneDPL falls back to a temporary-buffer wrap on every call
// (see docs/notes/onedpl_gpu_two_pitfalls.md, "Pitfall #2").

#include <sycl/sycl.hpp>

namespace pstl
{
	inline sycl::queue & onedpl_gpu_queue()
	{
		static sycl::queue q({ sycl::property::queue::in_order() });
		return q;
	}
} // namespace pstl

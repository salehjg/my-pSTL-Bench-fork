#pragma once

// Persistent sycl::buffer wrapper used in the NO_USM ONEDPL_GPU mode.
//
// _utils.h files lift one of these views over the host vector before the
// timed loop and let it destruct when the loop ends.  Inside the loop the
// view's begin()/end() return oneapi::dpl buffer iterators, so oneDPL kernels
// reuse the same device-side buffer across iterations rather than wrapping
// host iterators in a fresh temp buffer per call.
//
// See docs/notes/onedpl_gpu_fix_plan.md (Section C).

#ifndef PSTL_BENCH_ONEDPL_GPU_NO_USM
#  error "onedpl_gpu_buf_view.h is only valid under PSTL_BENCH_ONEDPL_GPU_NO_USM"
#endif

#include <cstddef>
#include <vector>

#include <oneapi/dpl/iterator>
#include <sycl/sycl.hpp>

namespace pstl
{
	template<typename T>
	struct onedpl_gpu_buf_view
	{
		sycl::buffer<T, 1> buf;

		template<typename Alloc>
		explicit onedpl_gpu_buf_view(std::vector<T, Alloc> & v)
		    : buf(v.data(), sycl::range<1>(v.size()))
		{}

		auto begin() { return oneapi::dpl::begin(buf); }
		auto end() { return oneapi::dpl::end(buf); }

		std::size_t size() const noexcept { return buf.size(); }
	};
} // namespace pstl

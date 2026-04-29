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

#ifdef __ACPP__
// AdaptiveCpp's clang doesn't define SYCL_LANGUAGE_VERSION as a built-in
// compiler macro the way icpx does. The macro is only set once
// <sycl/sycl.hpp> has been parsed. oneDPL's onedpl_config.h checks
// SYCL_LANGUAGE_VERSION the moment any <oneapi/dpl/*> header is first
// parsed to decide whether to load its SYCL-buffer-iterator support
// (pstl/hetero/dpcpp/sycl_iterator.h). If sycl.hpp is included AFTER
// the first oneDPL header, _ONEDPL_BACKEND_SYCL stays unset for the
// translation unit and `oneapi::dpl::begin(buf)`,
// `oneapi::dpl::execution::make_device_policy(...)`, etc. all silently
// disappear from the namespace.
//
// Pre-include sycl.hpp to anchor SYCL_LANGUAGE_VERSION before oneDPL
// is parsed in this header. icpx is unaffected because it defines
// SYCL_LANGUAGE_VERSION as a built-in via -fsycl, regardless of header
// order.
#include <sycl/sycl.hpp>
#endif

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

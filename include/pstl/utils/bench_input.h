#pragma once

// Memory-model adapter used by the per-algorithm _utils.h harnesses.
//
//   - NO_USM mode:  lifts a persistent sycl::buffer over the host vector for
//                   the lifetime of the bench_input. begin()/end() return
//                   oneDPL buffer iterators; host_view() returns a
//                   sycl::host_accessor for between-iteration host work.
//   - any other mode (USM, CPU backends): just a reference to the vector.
//                   begin()/end() forward to the vector; host_view() returns
//                   the vector itself.
//
// The API surface is identical in both modes, so _utils.h code stays free of
// #ifdefs.  The per-file pattern is:
//
//     auto data = pstl::generate_increment(...);
//     {
//         pstl::bench_input bench{data};
//         for (auto _ : state) {
//             pstl::wrap_timing(state, F, policy, bench, ...);
//         }
//     }   // bench dtor in NO_USM mode -> single device->host writeback
//     // verification runs here on host-fresh `data`
//
// See docs/notes/onedpl_gpu_fix_plan.md (Section C).

#include <cstddef>

#ifdef PSTL_BENCH_ONEDPL_GPU_NO_USM
#  include "pstl/utils/onedpl_gpu_buf_view.h"
#  include <sycl/sycl.hpp>
#endif

namespace pstl
{
#ifdef PSTL_BENCH_ONEDPL_GPU_NO_USM
	template<typename Vec>
	class bench_input
	{
	public:
		using value_type = typename Vec::value_type;

		explicit bench_input(Vec & v) : view_(v) {}

		// `view_` is mutable so begin()/end() are usable from const contexts
		// (some _std.h wrappers take their input as `const auto &`).  oneDPL
		// only mutates the underlying buffer when given a writable accessor,
		// so const-correctness here doesn't reflect anything load-bearing.
		auto        begin() const { return view_.begin(); }
		auto        end() const { return view_.end(); }
		auto        cbegin() const { return view_.begin(); }
		auto        cend() const { return view_.end(); }
		std::size_t size() const noexcept { return view_.size(); }

		// Returns a sycl::host_accessor exposing host-side iterators over the
		// underlying memory.  Use as `auto&& h = bench.host_view();` so its
		// lifetime spans the host work; it releases to the device on dtor.
		auto host_view() { return sycl::host_accessor<value_type, 1>{ view_.buf }; }

	private:
		mutable onedpl_gpu_buf_view<value_type> view_;
	};
#else
	template<typename Vec>
	class bench_input
	{
	public:
		using value_type = typename Vec::value_type;

		explicit bench_input(Vec & v) : v_(v) {}

		auto        begin() const { return v_.begin(); }
		auto        end() const { return v_.end(); }
		auto        cbegin() const { return v_.cbegin(); }
		auto        cend() const { return v_.cend(); }
		std::size_t size() const noexcept { return v_.size(); }

		Vec & host_view() { return v_; }

	private:
		Vec & v_;
	};
#endif
} // namespace pstl

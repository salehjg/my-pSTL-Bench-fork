#pragma once

// USM shared-memory allocator for std::vector.
// When plugged in as the allocator, iterators handed to oneDPL point to
// device-accessible memory, avoiding the per-call SYCL-buffer round trip
// that happens with plain host pointers.

#include <cstddef>
#include <new>
#include <sycl/sycl.hpp>

#include "pstl/utils/sycl_queue.h"

namespace pstl
{
	template<typename T>
	class usm_shared_allocator
	{
	public:
		using value_type = T;

		usm_shared_allocator() noexcept : q_(pstl::onedpl_gpu_queue()) {}
		explicit usm_shared_allocator(sycl::queue q) noexcept : q_(q) {}

		template<typename U>
		usm_shared_allocator(const usm_shared_allocator<U> & other) noexcept : q_(other.queue())
		{
		}

		T * allocate(std::size_t n)
		{
			T * ptr = sycl::malloc_shared<T>(n, q_);
			if (!ptr) { throw std::bad_alloc(); }
			return ptr;
		}

		void deallocate(T * p, std::size_t) noexcept { sycl::free(p, q_); }

		sycl::queue queue() const noexcept { return q_; }

		template<typename U>
		bool operator==(const usm_shared_allocator<U> & other) const noexcept
		{
			return q_ == other.queue();
		}

		template<typename U>
		bool operator!=(const usm_shared_allocator<U> & other) const noexcept
		{
			return !(*this == other);
		}

	private:
		sycl::queue q_;
	};
} // namespace pstl


#pragma once

#include <cmath>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_for_each
{
#ifndef PSTL_BENCH_DEFAULT_FOR_EACH_KERNEL_ITS
#define PSTL_BENCH_DEFAULT_FOR_EACH_KERNEL_ITS 1
#endif

	const auto kernel = [](auto & input, const auto Its) {
		volatile size_t I = Its;

		pstl::elem_t a{};

		for (auto i = 0; i < I; ++i)
		{
			a++;
		}

		input = a;
	};

	template<class Policy, class Function>
	static void benchmark_for_each_wrapper(benchmark::State & state, Function && f)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data = pstl::generate_increment(execution_policy, size);

		// Read environment variable PSTL_FOR_EACH_KERNEL_ITS
		std::size_t its = PSTL_BENCH_DEFAULT_FOR_EACH_KERNEL_ITS;

		const char * env_p = std::getenv("PSTL_BENCH_FOR_EACH_KERNEL_ITS");
		if (env_p) { its = std::strtoul(env_p, nullptr, 10); }

		{
			pstl::bench_input bench{ data };
			double total_stl_ns = 0;
			for (auto _ : state)
			{
				pstl::detail::per_iter_stl_ns() = 0;
				const auto _iter_t0 = std::chrono::high_resolution_clock::now();
				pstl::wrap_timing(state, std::forward<Function>(f), execution_policy, bench,
				                  [=](auto & elem) { kernel(elem, its); });
				const auto _iter_t1 = std::chrono::high_resolution_clock::now();
				total_stl_ns += pstl::detail::per_iter_stl_ns();
				state.SetIterationTime(std::chrono::duration<double>(_iter_t1 - _iter_t0).count());
			}
		} // bench dtor in NO_USM -> single device->host writeback to `data`

		auto verification_passed = pstl::verify([&]() {
			auto solution = pstl::generate_increment(execution_policy, size);
			std::for_each(std::begin(solution), std::end(solution), [=](auto & elem) { kernel(elem, its); });
			return pstl::are_equivalent(data, solution);
		});

		// stl_of_intrest_ns: per-iter average GPU dispatch (F()) time.
		state.counters["stl_of_intrest_ns"] = benchmark::Counter(
		    total_stl_ns, benchmark::Counter::kAvgIterations);
		// bytes_per_second: same value as the previous SetBytesProcessed-based
		// emission (bytes_per_iter / avg_stl_seconds). See
		// docs/notes/cpu_time_vs_real_time.md.
		const double _bytes_per_iter = static_cast<double>(pstl::computed_bytes(state, data)) / static_cast<double>(state.iterations());
		const double _avg_stl_seconds = (total_stl_ns / 1e9) / static_cast<double>(state.iterations());
		state.counters["bytes_per_second"] = _bytes_per_iter / _avg_stl_seconds;

		state.counters["kernel_its"] = static_cast<double>(its);
		pstl::set_verification_counter(state, verification_passed);
	}
} // namespace benchmark_for_each

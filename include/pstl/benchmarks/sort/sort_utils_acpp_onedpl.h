#pragma once

// Per-backend benchmark wrapper for ACPP_ONEDPL.
//
// Body identical to sort_utils.h: only per-iter prep is std::shuffle, which
// stays on host (oneDPL has no parallel shuffle). File exists so
// backend-specific changes have a place to land.
// See docs/notes/cpu_time_vs_real_time.md.

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_sort
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && f)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		static std::random_device rd;
		static std::mt19937       generator(rd());

		{
			pstl::bench_input bench{ input };
			double total_stl_ns = 0;
			for (auto _ : state)
			{
				pstl::detail::per_iter_stl_ns() = 0;
				const auto _iter_t0 = std::chrono::high_resolution_clock::now();
				{
					auto && h = bench.host_view();
					// shuffle stays on host: oneDPL has no parallel std::shuffle.
					std::shuffle(std::begin(h), std::end(h), generator);
				} // host_view dtor (NO_USM only) -> shuffle visible to device

				pstl::wrap_timing(state, std::forward<Function>(f), execution_policy, bench);
				const auto _iter_t1 = std::chrono::high_resolution_clock::now();
				total_stl_ns += pstl::detail::per_iter_stl_ns();
				state.SetIterationTime(std::chrono::duration<double>(_iter_t1 - _iter_t0).count());
			}
		}

		auto verification_result =
		    pstl::verify([&]() { return std::is_sorted(input.begin(), input.end()); });

		// stl_of_intrest_ns: per-iter average GPU dispatch (F()) time.
		state.counters["stl_of_intrest_ns"] = benchmark::Counter(
		    total_stl_ns, benchmark::Counter::kAvgIterations);
		// bytes_per_second: same value as the previous SetBytesProcessed-based
		// emission (bytes_per_iter / avg_stl_seconds). See
		// docs/notes/cpu_time_vs_real_time.md.
		const double _bytes_per_iter = static_cast<double>(pstl::computed_bytes(state, input)) / static_cast<double>(state.iterations());
		const double _avg_stl_seconds = (total_stl_ns / 1e9) / static_cast<double>(state.iterations());
		state.counters["bytes_per_second"] = _bytes_per_iter / _avg_stl_seconds;

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_sort

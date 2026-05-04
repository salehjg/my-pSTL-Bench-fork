#pragma once

// Per-backend benchmark wrapper for ACPP_ONEDPL.
//
// Body identical to partition_utils.h: only per-iter prep is std::shuffle,
// which stays on host (oneDPL has no parallel shuffle). File exists so
// backend-specific changes have a place to land.
// See docs/notes/cpu_time_vs_real_time.md.

#include <algorithm>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_partition
{
	const auto condition = [](const auto & i) {
		// Check if the number is even
		if constexpr (std::is_integral_v<decltype(i)>) { return i % 2 == 0; }
		else
		{
			return static_cast<int>(i) % 2 == 0;
		}
	};

	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

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
					std::shuffle(std::begin(h), std::end(h), std::mt19937(std::random_device()()));
				}

				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, condition);
				const auto _iter_t1 = std::chrono::high_resolution_clock::now();
				total_stl_ns += pstl::detail::per_iter_stl_ns();
				state.SetIterationTime(std::chrono::duration<double>(_iter_t1 - _iter_t0).count());
			}
		}

		auto verification_result =
		    pstl::verify([&]() { return std::is_partitioned(input.begin(), input.end(), condition); });

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
} // namespace benchmark_partition

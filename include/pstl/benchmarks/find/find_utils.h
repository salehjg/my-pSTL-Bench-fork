#pragma once

#include <cassert>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_find
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		// vector with values [0,size)
		auto input = pstl::generate_increment(execution_policy, size);

		// Seed with a fixed value for reproducibility
		const auto seed = 42;

		// Choose a random number between 1 and size
		std::minstd_rand engine(seed);

		// Random number generator
		std::uniform_int_distribution<size_t> gen(0, size - 1);

		pstl::elem_t   last_value{};
		std::ptrdiff_t last_offset = -1;
		{
			pstl::bench_input bench{ input };
			double total_stl_ns = 0;
			for (auto _ : state)
			{
				pstl::detail::per_iter_stl_ns() = 0;
				const auto _iter_t0 = std::chrono::high_resolution_clock::now();
				pstl::elem_t value;
				{
					auto && h        = bench.host_view();
					const auto index = gen(engine);
					value            = h[index];
				}
				last_value = value;

				const auto out_iter =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, value);
				last_offset = (out_iter == bench.end())
				                  ? std::ptrdiff_t{ -1 }
				                  : std::distance(bench.begin(), out_iter);
				const auto _iter_t1 = std::chrono::high_resolution_clock::now();
				total_stl_ns += pstl::detail::per_iter_stl_ns();
				state.SetIterationTime(std::chrono::duration<double>(_iter_t1 - _iter_t0).count());
			}
		}

		auto verification_passed = pstl::verify([&]() {
			if (last_offset < 0) { return false; }
			return pstl::are_equivalent(input[last_offset], last_value);
		});

		// stl_of_intrest_ns: per-iter average GPU dispatch (F()) time.
		state.counters["stl_of_intrest_ns"] = benchmark::Counter(
		    total_stl_ns, benchmark::Counter::kAvgIterations);
		// bytes_per_second: same value as the previous SetBytesProcessed-based
		// emission (bytes_per_iter / avg_stl_seconds). See
		// docs/notes/cpu_time_vs_real_time.md.
		const double _bytes_per_iter = static_cast<double>(pstl::computed_bytes(state, input)) / static_cast<double>(state.iterations());
		const double _avg_stl_seconds = (total_stl_ns / 1e9) / static_cast<double>(state.iterations());
		state.counters["bytes_per_second"] = _bytes_per_iter / _avg_stl_seconds;

		pstl::set_verification_counter(state, verification_passed);
	}
} // namespace benchmark_find

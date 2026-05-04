#pragma once

#include <cassert>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_any_of
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		static auto rd = std::random_device{};

		// Choose a random number between 1 and size
		static std::minstd_rand               engine{ rd() };
		std::uniform_int_distribution<size_t> gen(0, input.size() - 1);

		bool last_output = false;
		{
			pstl::bench_input bench{ input };
			double total_stl_ns = 0;
			for (auto _ : state)
			{
				pstl::detail::per_iter_stl_ns() = 0;
				const auto _iter_t0 = std::chrono::high_resolution_clock::now();
				const auto index = gen(engine);
				pstl::elem_t value;
				{
					auto && h = bench.host_view();
					value     = h[index];
				} // host_view dtor (NO_USM only) releases buffer back to device
				last_output = pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench,
				                                [=](const auto & val) { return val == value; });
				const auto _iter_t1 = std::chrono::high_resolution_clock::now();
				total_stl_ns += pstl::detail::per_iter_stl_ns();
				state.SetIterationTime(std::chrono::duration<double>(_iter_t1 - _iter_t0).count());
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = true;
			return pstl::are_equivalent(last_output, solution);
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

		pstl::set_verification_counter(state, verification_result);
	}

} // namespace benchmark_any_of

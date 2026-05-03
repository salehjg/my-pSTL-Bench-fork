#pragma once

// Per-backend benchmark wrapper for ACPP (acpp --acpp-stdpar, GPU device).
//
// Body identical to partial_sort_utils.h: only per-iter prep is std::shuffle,
// which stays on host (no parallel std::shuffle in C++17/20). File exists so
// backend-specific changes have a place to land.
// See docs/notes/cpu_time_vs_real_time.md.

#include <numeric>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_partial_sort
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		{
			pstl::bench_input bench{ input };
			const auto        half = bench.size() / 2;
			for (auto _ : state)
			{
				{
					auto && h = bench.host_view();
					// shuffle stays on host: no parallel std::shuffle in C++17/20.
					std::shuffle(std::begin(h), std::end(h), std::mt19937{ std::random_device{}() });
				}

				auto middle = bench.begin() + half;

				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench.begin(), middle,
				                  bench.end());
			}
		}

		auto verification_result =
		    pstl::verify([&]() { return std::is_sorted(input.begin(), input.begin() + (input.size() / 2)); });

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_partial_sort

#pragma once

// Per-backend benchmark wrapper for NVHPC_CUDA (nvc++ -stdpar=gpu).
//
// Runs the per-iteration "two sorted halves" prep on the GPU via
// std::sort(std::execution::par_unseq, ...) — nvc++'s stdpar offload picks
// it up. Replaces the serial host std::sort(...) used by the shared
// inplace_merge_utils.h. See docs/notes/cpu_time_vs_real_time.md.

#include <execution>
#include <numeric>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_inplace_merge
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

				std::sort(std::execution::par_unseq, bench.begin(), middle);
				std::sort(std::execution::par_unseq, middle, bench.end());

				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench.begin(), middle,
				                  bench.end());
			}
		}

		auto verification_passed =
		    pstl::verify([&]() { return std::is_sorted(input.begin(), input.end()); });

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_passed);
	}
} // namespace benchmark_inplace_merge

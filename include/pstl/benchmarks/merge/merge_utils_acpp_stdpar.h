#pragma once

// Per-backend benchmark wrapper for ACPP (acpp --acpp-stdpar, GPU device).
//
// Runs the per-iteration "two sorted halves" prep on the GPU via
// std::sort(std::execution::par_unseq, ...) — acpp's stdpar interceptor
// offloads it. Replaces the serial host std::sort(...) used by the shared
// merge_utils.h. See docs/notes/cpu_time_vs_real_time.md.

#include <execution>
#include <functional>
#include <numeric>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_merge
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input  = pstl::generate_increment(execution_policy, size);
		auto output = input;

		{
			pstl::bench_input bench_in{ input };
			pstl::bench_input bench_out{ output };
			const auto        half = bench_in.size() / 2;
			for (auto _ : state)
			{
				{
					auto && h = bench_in.host_view();
					// shuffle stays on host: no parallel std::shuffle in C++17/20.
					std::shuffle(std::begin(h), std::end(h), std::mt19937{ std::random_device{}() });
				}

				auto middle = bench_in.begin() + half;

				// 4-arg form with std::less<>{} works around acpp 25.10's stdpar
				// 3-arg sort template-deduction bug — see
				// docs/notes/acpp_stdpar_sort_template_bug.md.
				std::sort(std::execution::par_unseq, bench_in.begin(), middle, std::less<>{});
				std::sort(std::execution::par_unseq, middle, bench_in.end(), std::less<>{});

				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench_in.begin(), middle,
				                  middle, bench_in.end(), bench_out.begin());
			}
		}

		auto middle = input.begin() + (input.size() / 2);
		auto verification_result = pstl::verify([&]() {
			return std::is_sorted(output.begin(), output.end()) and
			       std::includes(output.begin(), output.end(), input.begin(), middle) and
			       std::includes(output.begin(), output.end(), middle, input.end());
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_merge

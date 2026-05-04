#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/host_parallel.h"
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
					auto && h        = bench_in.host_view();
					auto    h_middle = std::begin(h) + half;
					std::shuffle(std::begin(h), std::end(h), std::mt19937{ std::random_device{}() });
					pstl::host_sort(std::begin(h), h_middle);
					pstl::host_sort(h_middle, std::end(h));
				}

				auto middle = bench_in.begin() + half;

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

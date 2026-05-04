#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/host_parallel.h"
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
					auto && h        = bench.host_view();
					auto    h_middle = std::begin(h) + half;
					std::shuffle(std::begin(h), std::end(h), std::mt19937{ std::random_device{}() });
					pstl::host_sort(std::begin(h), h_middle);
					pstl::host_sort(h_middle, std::end(h));
				}

				auto middle = bench.begin() + half;

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

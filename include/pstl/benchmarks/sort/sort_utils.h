#pragma once

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
			for (auto _ : state)
			{
				{
					auto && h = bench.host_view();
					std::shuffle(std::begin(h), std::end(h), generator);
				} // host_view dtor (NO_USM only) -> shuffle visible to device

				pstl::wrap_timing(state, std::forward<Function>(f), execution_policy, bench);
			}
		}

		auto verification_result =
		    pstl::verify([&]() { return std::is_sorted(input.begin(), input.end()); });

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_sort

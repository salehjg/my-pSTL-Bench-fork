#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_is_sorted
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		bool last_output = false;
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				last_output = pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench);
			}
		}

		auto verification_passed = pstl::verify([&]() {
			const auto solution = std::is_sorted(input.begin(), input.end());
			return pstl::are_equivalent(last_output, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_passed);
	}
} // namespace benchmark_is_sorted

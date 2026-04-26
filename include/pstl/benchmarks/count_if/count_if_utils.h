#pragma once

#include <algorithm>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_count_if
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

		using count_t = decltype(std::count_if(input.begin(), input.end(), condition));
		count_t last_output{};
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				last_output =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, condition);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = std::count_if(input.begin(), input.end(), condition);
			return pstl::are_equivalent(last_output, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_count_if
